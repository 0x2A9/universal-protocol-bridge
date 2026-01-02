#include <stdio.h>
#include "system_init.h"
#include "device.hpp"
#include "usb_device.h"
#include "usbd_cdc_if.h"

void BoardLedController::ToggleInfo(void) {
  HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8);
}

BoardUsb::BoardUsb() {
  /* Avoid multiple instances */
  if (instance_ != nullptr) Error_Handler();

  instance_ = this;
}

/* May return nullptr */
BoardUsb* BoardUsb::TryInstance() {
  return instance_;
}

bool BoardUsb::IsReady(void) const {
  return USB_GetDeviceHandle()->dev_state == USBD_STATE_CONFIGURED;
}

uint8_t BoardUsb::Transmit(uint8_t* buf, uint16_t len) {
  return CDC_Transmit_FS(buf, len);
}

uint16_t BoardUsb::ItemCount() const {
  return (head_ - tail_) & kBufMask;
}

uint16_t BoardUsb::PopRx(uint8_t* dst, uint32_t len) {
  uint16_t n = ItemCount();
  if (n > len) n = len;

  for (uint32_t i = 0; i < n; i++) {
    dst[i] = buf_[tail_];
    tail_ = (tail_ + 1) & kBufMask;
  }
  return n;
}

void BoardUsb::PushRx(const uint8_t* data, uint32_t len) {
  // Push bytes
  for (uint32_t i = 0; i < len; i++) {
    uint16_t next = (head_ + 1) & kBufMask;
    if (next == tail_) break;

    buf_[head_] = data[i];
    head_ = next;
  }

  // Append newline at end of message
  uint16_t next = (head_ + 1) & kBufMask;
  if (next != tail_) {
    buf_[head_] = '\n';
    head_ = next;
  } else {
    // No space to append -> replace last byte with '\n'
    // (only if message had at least 1 byte pushed)
    if (head_ != tail_) {
      uint16_t last = (head_ - 1) & kBufMask;
      buf_[last] = '\n';
    }
  }
}

void Device::Init(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
}

void Device::Run(void) {
  if (!usb_.IsReady()) return;

  /* Simple USB test */
  leds_.ToggleInfo();

  char buf[64];
  int n = usb_.PopRx((uint8_t*)buf, sizeof(buf));

  if (n > 0) {
    usb_.Transmit((uint8_t*)buf, (uint16_t)n);
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

extern "C" void BoardUsb_OnRx(const uint8_t* data, uint16_t len) {
  if (BoardUsb* u = BoardUsb::TryInstance()) {
    u->PushRx(data, len);
  }
}
