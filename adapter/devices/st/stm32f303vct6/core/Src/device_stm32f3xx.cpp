#include <stdio.h>
#include "system_init.h"
#include "device.hpp"
#include "usb_device.h"
#include "usbd_cdc_if.h"

void BoardLedController::ToggleInfo(void) {
  HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8);
}

bool BoardUsb::IsReady(void) const {
  return USB_GetDeviceHandle()->dev_state == USBD_STATE_CONFIGURED;
}

uint8_t BoardUsb::Transmit(uint8_t* buf, uint16_t len) {
  return CDC_Transmit_FS(buf, len);
}

void Device::Init(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
}

void Device::Run(void) {
  if (!this->usb_.IsReady()) return;

  this->leds_.ToggleInfo();

  /* Simple USB test */
  char buf[64];
  uint32_t t = HAL_GetTick();

  int n = snprintf(buf, sizeof(buf), "%lu\r\n", (unsigned long)t);

  if (n > 0) {
    this->usb_.Transmit((uint8_t*)buf, (uint16_t)n);
  }
}

void Device::DelayMs(uint32_t ms) {
  HAL_Delay(ms);
}
