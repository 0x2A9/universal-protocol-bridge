#include <stdio.h>
#include "board.h"
#include "device.hpp"
#include "usb_device.h"
#include "usbd_cdc_if.h"

static uint8_t dcp_tx[kDcpMaxFrameSize];

static constexpr uint8_t kFwVersionMajor = 1;
static constexpr uint8_t kFwVersionMinor = 0;
static constexpr uint8_t kFwVersionPatch = 0;
static constexpr uint8_t kSupportedInterfaceMask = 0x01; /* bit0 = UART */

void LedController::ToggleInfo(void) {
  HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8);
}

void LedController::SetWarn(void) {
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);
}

void LedController::ResetWarn(void) {
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
}

Usb::Usb(void) {
  if (instance_ != nullptr) Error_Handler();

  instance_ = this;
}

Usb *Usb::TryInstance(void) {
  return instance_;
}

bool Usb::Init(void) {
  MX_USB_DEVICE_Init();
  return true;
}

bool Usb::IsReady(void) const {
  return USB_GetDeviceHandle()->dev_state == USBD_STATE_CONFIGURED;
}

bool Usb::EnqueueTx(const uint8_t *src, const uint16_t len) {
  if (tx_buf_.Free() < len) return false;
  tx_buf_.Push(src, len);
  
  return true;
}

void Usb::ProcessTx(void) {
  if (!IsReady()) return;

  uint16_t avail = tx_buf_.Count();
  if (avail == 0) return;

  uint8_t tmp[64];
  uint16_t n = (avail > sizeof(tmp)) ?
               (uint16_t)sizeof(tmp) :
               avail;

  n = tx_buf_.Peek(tmp, n);
  if (n == 0) return;

  uint8_t st = CDC_Transmit_FS(tmp, n);
  if (st == USBD_OK) {
    tx_buf_.Drop(n); // Commit only on success
  }
}

bool Usb::EnqueueRx(const uint8_t *src, const uint16_t len) {
  if (rx_buf_.Free() < len) return false;
  rx_buf_.Push(src, len);

  return true;
}

uint16_t Usb::DequeueRx(uint8_t *dst, const uint16_t len) {
  return rx_buf_.Pop(dst, len);
}

Uart::Uart(void) {
  if (instance_ != nullptr) Error_Handler();

  instance_ = this;
}

Uart *Uart::TryInstance(void) {
  return instance_;
}

bool Uart::Init(void) {
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {};

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  MX_USART2_UART_Init();
  StartRx();

  return true;
}

bool Uart::Reconfigure(const UartConfig &cfg) {
  HAL_UART_DeInit(&huart2);

  huart2.Instance = USART2;
  huart2.Init.BaudRate = cfg.baud;

  switch (cfg.data_bits) {
    case 9:  huart2.Init.WordLength = UART_WORDLENGTH_9B; break;
    default: huart2.Init.WordLength = UART_WORDLENGTH_8B; break;
  }

  huart2.Init.StopBits = (cfg.stop_bits >= 2) ? UART_STOPBITS_2 : UART_STOPBITS_1;

  switch (cfg.parity) {
    case 1:  huart2.Init.Parity = UART_PARITY_EVEN; break;
    case 2:  huart2.Init.Parity = UART_PARITY_ODD; break;
    default: huart2.Init.Parity = UART_PARITY_NONE; break;
  }

  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  if (HAL_UART_Init(&huart2) != HAL_OK) return false;

  StartRx();

  return true;
}

void Uart::StartRx(void) {
  HAL_UART_Receive_IT(&huart2, rx_tmp_, sizeof(rx_tmp_));
}

uint8_t Uart::Transmit(uint8_t *src, const uint16_t len) {
  return HAL_UART_Transmit(&huart2, src, len, 10);
}

bool Uart::CopyRx(void) {
  if (!EnqueueRx(rx_tmp_, sizeof(rx_tmp_))) return false;
  is_new_rx_data_ = true;

  return true;
}

bool Uart::EnqueueRx(const uint8_t *src, const uint16_t len) {
  if (rx_buf_.Free() < len) return false;
  rx_buf_.Push(src, len);

  return true;
}

uint16_t Uart::DequeueRx(uint8_t *dst, const uint16_t len) {
  return rx_buf_.Pop(dst, len);
}

bool Uart::IsNewRxData(void) {
  return is_new_rx_data_;
}
void Uart::ClearNewRxDataFlag(void) {
  is_new_rx_data_ = false;
}

void Device::Init(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  usb_.Init();
  uart_.Init();
}

void Device::SendErr(DcpInterface iface, uint8_t txn_id, DcpError err) {
  uint8_t code = static_cast<uint8_t>(err);
  uint16_t n = DcpEncode(dcp_tx, DcpCmd::kErr, iface, txn_id, &code, 1);
  
  usb_.EnqueueTx(dcp_tx, n);
}

void Device::SendUartCfg(uint8_t txn_id) {
  uint8_t payload[7];
  payload[0] = uint8_t(uart_cfg_.baud >> 24);
  payload[1] = uint8_t(uart_cfg_.baud >> 16);
  payload[2] = uint8_t(uart_cfg_.baud >> 8);
  payload[3] = uint8_t(uart_cfg_.baud);
  payload[4] = uart_cfg_.data_bits;
  payload[5] = uart_cfg_.stop_bits;
  payload[6] = uart_cfg_.parity;

  uint16_t n = DcpEncode(dcp_tx, DcpCmd::kCfg, DcpInterface::kUart, txn_id,
                          payload, sizeof(payload));
  usb_.EnqueueTx(dcp_tx, n);
}

void Device::SendSystemCfg(uint8_t txn_id) {
  uint8_t payload[4] = {kFwVersionMajor, kFwVersionMinor, kFwVersionPatch,
                         kSupportedInterfaceMask};

  uint16_t n = DcpEncode(dcp_tx, DcpCmd::kCfg, DcpInterface::kSystem, txn_id,
                          payload, sizeof(payload));
  usb_.EnqueueTx(dcp_tx, n);
}

void Device::ApplyUartCfg(const uint8_t *payload, uint16_t len) {
  if (len < 7) return;

  uart_cfg_.baud = (uint32_t(payload[0]) << 24) | (uint32_t(payload[1]) << 16) |
                   (uint32_t(payload[2]) << 8) | uint32_t(payload[3]);
  uart_cfg_.data_bits = payload[4];
  uart_cfg_.stop_bits = payload[5];
  uart_cfg_.parity = payload[6];

  uart_.Reconfigure(uart_cfg_);
}

void Device::HandleRun(const DcpFrame &frame) {
  if (frame.interface == DcpInterface::kUart) {
    uart_.Transmit(const_cast<uint8_t *>(frame.payload), frame.len);

    uint16_t n = DcpEncode(dcp_tx, DcpCmd::kAck, DcpInterface::kUart,
                            frame.txn_id, nullptr, 0);
    usb_.EnqueueTx(dcp_tx, n);

    return;
  }

  SendErr(frame.interface, frame.txn_id, DcpError::kUnsupportedInterface);
}

void Device::HandleGetCfg(const DcpFrame &frame) {
  switch (frame.interface) {
    case DcpInterface::kUart:   SendUartCfg(frame.txn_id); break;
    case DcpInterface::kSystem: SendSystemCfg(frame.txn_id); break;
    default:
      SendErr(frame.interface, frame.txn_id, DcpError::kUnsupportedInterface);

      break;
  }
}

void Device::HandleSetCfg(const DcpFrame &frame) {
  switch (frame.interface) {
    case DcpInterface::kUart:
      ApplyUartCfg(frame.payload, frame.len);
      SendUartCfg(frame.txn_id);

      break;

    default:
      SendErr(frame.interface, frame.txn_id, DcpError::kUnsupportedInterface);

      break;
  }
}

void Device::HandleReset(const DcpFrame &frame) {
  switch (frame.interface) {
    case DcpInterface::kUart:
      uart_.Reconfigure(uart_cfg_);

      break;

    case DcpInterface::kSystem: {
      uint16_t n = DcpEncode(dcp_tx, DcpCmd::kAck, DcpInterface::kSystem,
                              frame.txn_id, nullptr, 0);
      usb_.EnqueueTx(dcp_tx, n);
      usb_.ProcessTx();
      NVIC_SystemReset();

      return;
    }

    default:
      SendErr(frame.interface, frame.txn_id, DcpError::kUnsupportedInterface);

      return;
  }

  uint16_t n = DcpEncode(dcp_tx, DcpCmd::kAck, frame.interface, frame.txn_id,
                          nullptr, 0);
  usb_.EnqueueTx(dcp_tx, n);
}

void Device::HandleFrame(const DcpFrame &frame) {
  switch (frame.cmd) {
    case DcpCmd::kRun:    HandleRun(frame); break;
    case DcpCmd::kGetCfg: HandleGetCfg(frame); break;
    case DcpCmd::kSetCfg: HandleSetCfg(frame); break;
    case DcpCmd::kReset:  HandleReset(frame); break;
    default:
      SendErr(frame.interface, frame.txn_id, DcpError::kMalformedFrame);

      break;
  }
}

void Device::Run(void) {
  if (!usb_.IsReady()) return;

  leds_.ResetWarn();
  leds_.ToggleInfo();

  uint8_t chunk[64];
  uint16_t n = usb_.DequeueRx(chunk, sizeof(chunk));

  if (n > 0) {
    dcp_rx_.Feed(chunk, n);
  }

  DcpFrame frame;
  DcpError err;
  DcpPopResult r;

  while ((r = dcp_rx_.PopFrame(frame, err)) != DcpPopResult::kNone) {
    if (r == DcpPopResult::kProtocolError) {
      SendErr(frame.interface, frame.txn_id, err);

      continue;
    }

    HandleFrame(frame);
  }

  if (uart_.IsNewRxData()) {
    leds_.SetWarn();
    uint8_t rxbuf[64];
    uint16_t rn = uart_.DequeueRx(rxbuf, sizeof(rxbuf));
    if (rn > 0) {
      uint16_t fn = DcpEncode(dcp_tx, DcpCmd::kData, DcpInterface::kUart, 0,
                               rxbuf, rn);

      usb_.EnqueueTx(dcp_tx, fn);
    }

    uart_.ClearNewRxDataFlag();
  }

  usb_.ProcessTx();

  static uint32_t last_heartbeat = 0;
  uint32_t t = HAL_GetTick();
  if (t - last_heartbeat >= 1000) {
    last_heartbeat = t;
    
    uint16_t hn = DcpEncode(dcp_tx, DcpCmd::kHeartbeat, DcpInterface::kSystem,
                             0, nullptr, 0);

    usb_.EnqueueTx(dcp_tx, hn);
  }

  usb_.ProcessTx();
}

void Device::DelayMs(const uint32_t ms) {
  HAL_Delay(ms);
}
