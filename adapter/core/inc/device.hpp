#ifndef ADAPTER_CORE_INC_DEVICE_HPP
#define ADAPTER_CORE_INC_DEVICE_HPP

#include <stdint.h>
#include "queue.hpp"

#ifdef __cplusplus
extern "C" {
#endif

class LedController {
 public:
  virtual void ToggleInfo(void) = 0;
  virtual void SetWarn(void) = 0;
  virtual void ResetWarn(void) = 0;

  virtual ~LedController() = default;
};

class Usb {
 public:
  virtual bool IsReady(void) const = 0;
  virtual uint8_t Transmit(uint8_t *buf, uint16_t len) = 0;

  virtual ~Usb() = default;
};

class Uart {
 public:
  explicit Uart(void);
  static Uart *TryInstance();

  bool Init(void);
  uint8_t Transmit(uint8_t *buf, uint16_t len);

  void StartRx(void);
  uint16_t PopRx(uint8_t *dst, uint32_t len);
  bool PushRx(const uint8_t *data, uint32_t len);
  bool CopyRx(void);

  bool IsNewRxData(void);
  void ClearNewRxDataFlag(void);

 private:
  static inline Uart *instance_  = nullptr;

  bool is_new_rx_data_ = false;

  uint8_t rx_tmp_[8];
  Queue rx_buf_;
};

class BoardLedController : public LedController {
 public:
  void ToggleInfo(void) override;
  void SetWarn(void) override;
  void ResetWarn(void) override;
};

class BoardUsb : public Usb {
 public:
  explicit BoardUsb(void);
  static BoardUsb *TryInstance(void);

  bool IsReady(void) const override;
  uint8_t Transmit(uint8_t *buf, uint16_t len) override;

  uint16_t PopRx(uint8_t *dst, uint32_t len);
  bool PushRx(const uint8_t *data, uint32_t len);

 private:
  static inline BoardUsb *instance_  = nullptr;
  Queue rx_buf_;
};

class Device {
 public:
  explicit Device(BoardLedController &lc, BoardUsb &usb, Uart &uart)
      : leds_(lc), usb_(usb), uart_(uart) {}

  void Init(void);
  void Run(void);
  static void DelayMs(uint32_t ms);

 private:
  BoardLedController &leds_;
  BoardUsb &usb_;
  Uart &uart_;
};

#ifdef __cplusplus
}
#endif

#endif // ADAPTER_CORE_INC_DEVICE_HPP