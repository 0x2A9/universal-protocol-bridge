#include <stdio.h>
#include "board.h"
#include "device.hpp"
#include "usb_device.h"
#include "usbd_cdc_if.h"

static volatile bool uart_rx_ready = false;

Uart::Uart() {
  /* Avoid multiple instances */
  if (instance_ != nullptr) Error_Handler();

  instance_ = this;
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

uint8_t Uart::Transmit(uint8_t *buf, uint16_t len) {
  return HAL_UART_Transmit(&huart2, buf, len, 10);
}

void Uart::StartRx(void) {
  HAL_UART_Receive_IT(&huart2, rx_tmp_, sizeof(rx_tmp_));
}

uint16_t Uart::PopRx(uint8_t *dst, uint32_t len) {
  return rx_buf_.Pop(dst, len);
}

bool Uart::PushRx(const uint8_t *data, uint32_t len) {
  if (rx_buf_.Free() >= len) {
    rx_buf_.Push(data, len);
    return true;
  }

  return false;
}

bool Uart::CopyRx(void) {
  if (PushRx(rx_tmp_, sizeof(rx_tmp_))) {
    is_new_rx_data_ = true;
    return true;
  }

  return false;
}

bool Uart::IsNewRxData(void) {
  return is_new_rx_data_;
}
void Uart::ClearNewRxDataFlag(void) {
  is_new_rx_data_ = false;
}

void BoardLedController::ToggleInfo(void) {
  HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8);
}

void BoardLedController::SetWarn(void) {
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);
}

void BoardLedController::ResetWarn(void) {
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
}

BoardUsb::BoardUsb() {
  /* Avoid multiple instances */
  if (instance_ != nullptr) Error_Handler();

  instance_ = this;
}

/* May return nullptr */
BoardUsb *BoardUsb::TryInstance() {
  return instance_;
}

bool BoardUsb::IsReady(void) const {
  return USB_GetDeviceHandle()->dev_state == USBD_STATE_CONFIGURED;
}

uint8_t BoardUsb::Transmit(uint8_t *buf, uint16_t len) {
  uint8_t status = CDC_Transmit_FS(buf, len);

  while (status == USBD_BUSY) {
    status = CDC_Transmit_FS(buf, len);
  }

  return status;
}

uint16_t BoardUsb::PopRx(uint8_t *dst, uint32_t len) {
  return rx_buf_.Pop(dst, len);
}

bool BoardUsb::PushRx(const uint8_t *data, uint32_t len) {
  if (rx_buf_.Free() >= len) {
    rx_buf_.Push(data, len);
    return true;
  }

  return false;
}

void Device::Init(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();

  uart_.Init();
}

void Device::Run(void) {
  /* Simple USB-UART test */
  if (!usb_.IsReady()) return;

  leds_.ResetWarn();
  leds_.ToggleInfo();

  int n = 0;

  char buf[64];
  n = usb_.PopRx((uint8_t*)buf, sizeof(buf));

  if (n > 0) {
    uart_.Transmit((uint8_t*)buf, (uint16_t)n);
  }

  if (uart_.IsNewRxData()) {
    leds_.SetWarn();
    uint8_t uart_data[64];
    n = uart_.PopRx(uart_data, sizeof(uart_data));
    usb_.Transmit((uint8_t*)uart_data, (uint16_t)n);
    uart_.ClearNewRxDataFlag();
  }

  uint32_t t = HAL_GetTick();

  n = snprintf(buf, sizeof(buf), "%lu\n", (unsigned long)t);

  if (n > 0) {
    usb_.Transmit((uint8_t*)buf, (uint16_t)n);
  }
}

void Device::DelayMs(uint32_t ms) {
  HAL_Delay(ms);
}
