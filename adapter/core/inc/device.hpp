#ifndef ADAPTER_CORE_INC_DEVICE_HPP
#define ADAPTER_CORE_INC_DEVICE_HPP

#include <stdint.h>
#include "queue.hpp"

#ifdef __cplusplus
extern "C" {
#endif

class LedController {
 public:
  void ToggleInfo(void);
  void SetWarn(void);
  void ResetWarn(void);
};

class Usb {
 public:
  explicit Usb(void);
  static Usb *TryInstance(void);

  bool IsReady(void) const;
  uint8_t Transmit(uint8_t *buf, uint16_t len);

  uint16_t PopRx(uint8_t *dst, uint32_t len);
  bool PushRx(const uint8_t *data, uint32_t len);

 private:
  static inline Usb *instance_ = nullptr;
  Queue rx_buf_;
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

class Device {
 public:
  explicit Device(LedController &lc, Usb &usb, Uart &uart)
      : leds_(lc), usb_(usb), uart_(uart) {}

  void Init(void);
  void Run(void);
  static void DelayMs(uint32_t ms);

 private:
  LedController &leds_;
  Usb &usb_;
  Uart &uart_;
};

#ifdef __cplusplus
}
#endif

#endif // ADAPTER_CORE_INC_DEVICE_HPP