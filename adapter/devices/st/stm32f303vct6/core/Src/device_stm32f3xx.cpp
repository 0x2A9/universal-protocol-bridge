#include <stdio.h>
#include "board.h"
#include "device.hpp"
#include "controllers/led_controller.hpp"
#include "controllers/peripherals_controller.hpp"
#include "dcp/dcp_handler.hpp"
#include "usb_device.h"
#include "usbd_cdc_if.h"

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

/* ------------------------------------------------------------------- */
/* Device                                                               */
/* ------------------------------------------------------------------- */

void Device::Init(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  dcp_.Init();
  peripherals_.Init();
}

void Device::Run(void) {
  if (!dcp_.IsReady()) return;

  leds_.ResetWarn();
  leds_.ToggleInfo();

  DcpHandler::ProcessResult result = dcp_.Process();
  if (result.uart_rx_processed) leds_.SetWarn();

  dcp_.Tick(HAL_GetTick());
}

void Device::DelayMs(const uint32_t ms) {
  HAL_Delay(ms);
}
