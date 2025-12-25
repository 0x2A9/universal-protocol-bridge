#ifndef ADAPTER_CORE_INC_DEVICE_HPP
#define ADAPTER_CORE_INC_DEVICE_HPP

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

class LedController {
 public:
  virtual void ToggleInfo() = 0;
  virtual ~LedController() = default;
};

class Usb {
 public:
  virtual bool IsReady(void) const = 0;
  virtual uint8_t Transmit(uint8_t* buf, uint16_t len) = 0;
  virtual ~Usb() = default;
};

class BoardLedController : public LedController {
 public:
  void ToggleInfo(void) override;
};

class BoardUsb : public Usb {
 public:
  bool IsReady(void) const override;
  uint8_t Transmit(uint8_t *buf, uint16_t len) override;
};

class Device {
 public:
  explicit Device(LedController &lc, Usb &usb)
      : leds_(lc), usb_(usb) {}

  static void Init(void);
  void Run(void);
  static void DelayMs(uint32_t ms);

 private:
  LedController &leds_;
  Usb &usb_;
};

#ifdef __cplusplus
}
#endif

#endif // ADAPTER_CORE_INC_DEVICE_HPP