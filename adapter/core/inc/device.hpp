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
  explicit BoardUsb();

  static BoardUsb* TryInstance();

  bool IsReady(void) const override;
  uint8_t Transmit(uint8_t *buf, uint16_t len) override;

  uint16_t ItemCount() const;
  uint16_t PopRx(uint8_t* dst, uint32_t len);
  void PushRx(const uint8_t* data, uint32_t len);

 private:
  /* Should be the power of two */
  static constexpr uint16_t kBufSize = 512;
  static constexpr uint16_t kBufMask = kBufSize - 1;
  static inline BoardUsb* instance_  = nullptr;

  volatile uint16_t head_ = 0;
  volatile uint16_t tail_ = 0;
  uint8_t buf_[kBufSize] {};
};

class Device {
 public:
  explicit Device(BoardLedController &lc, BoardUsb &usb)
      : leds_(lc), usb_(usb) {}

  static void Init(void);
  void Run(void);
  static void DelayMs(uint32_t ms);

 private:
  BoardLedController &leds_;
  BoardUsb &usb_;
};

#ifdef __cplusplus
}
#endif

#endif // ADAPTER_CORE_INC_DEVICE_HPP