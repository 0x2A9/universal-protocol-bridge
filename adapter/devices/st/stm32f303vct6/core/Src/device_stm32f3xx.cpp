#include <stdio.h>
#include "board.h"
#include "device.hpp"
#include "usb_device.h"
#include "usbd_cdc_if.h"

static char buf[64];

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
  /* Avoid multiple instances */
  if (instance_ != nullptr) Error_Handler();

  instance_ = this;
}

/* May return nullptr */
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

  /* CDC FS packet is 64 bytes max */
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
  /* If BUSY: do nothing, try again next loop */
}

bool Usb::EnqueueRx(const uint8_t *src, const uint16_t len) {
  if (rx_buf_.Free() < len) return false;
  rx_buf_.Push(src, len);
  return true;
}

uint16_t Usb::DequeueRx(uint8_t *dst, const uint16_t len) {
  return rx_buf_.Pop(dst, len);
}

I2c::I2c(void) {
  /* Avoid multiple instances */
  if (instance_ != nullptr) Error_Handler();

  instance_ = this;
}

/* May return nullptr */
I2c *I2c::TryInstance(void) {
  return instance_;
}

Uart::Uart(void) {
  /* Avoid multiple instances */
  if (instance_ != nullptr) Error_Handler();

  instance_ = this;
}

bool I2c::Init(void) {
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {};

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
  PeriphClkInit.I2c2ClockSelection   = RCC_I2C2CLKSOURCE_HSI;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  MX_I2C2_Init();

  return true;
}

/* May return nullptr */
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

/* Rearms UART data reception */
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
  i2c_.Init();
  uart_.Init();
}

void Device::Run(void) {
  /* Simple USB-UART test */
  if (!usb_.IsReady()) return;

  leds_.ResetWarn();
  leds_.ToggleInfo();

  int n = 0;

  n = usb_.DequeueRx((uint8_t*)buf, sizeof(buf));

  if (n > 0) {
    usb_.EnqueueTx((uint8_t*)buf, (uint16_t)n);
  }

  if (uart_.IsNewRxData()) {
    leds_.SetWarn();
    n = uart_.DequeueRx((uint8_t*)buf, sizeof(buf));
    usb_.EnqueueTx((uint8_t*)buf, (uint16_t)n);
    uart_.ClearNewRxDataFlag();
  }

  usb_.ProcessTx();

  uint32_t t = HAL_GetTick();

  n = snprintf(buf, sizeof(buf), "%lu\n", (unsigned long)t);

  if (n > 0) {
    usb_.EnqueueTx((uint8_t*)buf, (uint16_t)n);
  }

  usb_.ProcessTx();
}

void Device::DelayMs(const uint32_t ms) {
  HAL_Delay(ms);
}
