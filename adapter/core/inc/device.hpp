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

  bool Init(void);
  bool IsReady(void) const;

  bool EnqueueTx(const uint8_t *src, const uint16_t len);
  void ProcessTx(void);
  bool EnqueueRx(const uint8_t *src, const uint16_t len);
  uint16_t DequeueRx(uint8_t *dst, const uint16_t len);

 private:
  static inline Usb *instance_ = nullptr;

  Queue rx_buf_;
  Queue tx_buf_;
};

class Uart {
 public:
  explicit Uart(void);
  static Uart *TryInstance(void);

  bool Init(void);
  void StartRx(void);

  uint8_t Transmit(uint8_t *src, const uint16_t len);
  bool CopyRx(void);
  bool EnqueueRx(const uint8_t *src, const uint16_t len);
  uint16_t DequeueRx(uint8_t *dst, const uint16_t len);

  bool IsNewRxData(void);
  void ClearNewRxDataFlag(void);

 private:
  static constexpr uint16_t kRxTmpBufSize = 8;
  static inline Uart *instance_ = nullptr;

  bool is_new_rx_data_ = false;

  /* UART Rx ISR triggered as soon as it is filled */
  uint8_t rx_tmp_[kRxTmpBufSize];
  Queue rx_buf_;
};

class Device {
 public:
  explicit Device(LedController &lc, Usb &usb, Uart &uart)
      : leds_(lc), usb_(usb), uart_(uart) {}

  void Init(void);
  void Run(void);
  static void DelayMs(const uint32_t ms);

 private:
  LedController &leds_;
  Usb &usb_;
  Uart &uart_;
};

#ifdef __cplusplus
}
#endif

#endif // ADAPTER_CORE_INC_DEVICE_HPP