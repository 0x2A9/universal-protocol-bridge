#ifndef ADAPTER_CORE_INC_DEVICE_HPP
#define ADAPTER_CORE_INC_DEVICE_HPP

#include <stdint.h>
#include "queue.hpp"

#ifdef __cplusplus
extern "C" {
#endif

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

struct UartConfig {
  uint32_t baud = 115200;
  uint8_t data_bits = 8;
  uint8_t stop_bits = 1;
  uint8_t parity = 0; /* 0=None, 1=Even, 2=Odd */
};

class Uart {
 public:
  explicit Uart(void);
  static Uart *TryInstance(void);

  bool Init(void);
  void StartRx(void);
  bool Reconfigure(const UartConfig &cfg);

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

/* LedController lives in controllers/led_controller.hpp, PeripheralsController
 * in controllers/peripherals_controller.hpp, DcpHandler in
 * dcp/dcp_handler.hpp (DcpSender is DcpHandler's own implementation detail,
 * not referenced here) -- Device only needs a reference to each, so a
 * forward declaration is enough and keeps this header free of a dependency
 * cycle (peripherals_controller.hpp depends on the Usb/Uart/UartConfig
 * types declared above). */
class LedController;
class DcpHandler;
class PeripheralsController;

class Device {
 public:
  explicit Device(LedController &lc, DcpHandler &dcp, PeripheralsController &peripherals)
      : leds_(lc), dcp_(dcp), peripherals_(peripherals) {}

  void Init(void);
  void Run(void);
  static void DelayMs(const uint32_t ms);

 private:
  LedController &leds_;
  DcpHandler &dcp_;
  PeripheralsController &peripherals_;
};

#ifdef __cplusplus
}
#endif

#endif // ADAPTER_CORE_INC_DEVICE_HPP
