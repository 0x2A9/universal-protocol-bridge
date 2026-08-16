#ifndef ADAPTER_CORE_INC_DEVICE_HPP
#define ADAPTER_CORE_INC_DEVICE_HPP

#include <stdint.h>
#include "queue.hpp"
#include "dcp.hpp"

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

class Device {
 public:
  explicit Device(LedController &lc, Usb &usb, Uart &uart)
      : leds_(lc), usb_(usb), uart_(uart) {}

  void Init(void);
  void Run(void);
  static void DelayMs(const uint32_t ms);

 private:
  void HandleFrame(const DcpFrame &frame);
  void HandleRun(const DcpFrame &frame);
  void HandleGetCfg(const DcpFrame &frame);
  void HandleSetCfg(const DcpFrame &frame);
  void HandleReset(const DcpFrame &frame);

  void SendErr(DcpInterface iface, uint8_t txn_id, DcpError err);
  void SendUartCfg(uint8_t txn_id);
  void SendSystemCfg(uint8_t txn_id);
  void ApplyUartCfg(const uint8_t *payload, uint16_t len);

  LedController &leds_;
  Usb &usb_;
  Uart &uart_;

  DcpParser dcp_rx_;
  UartConfig uart_cfg_;
};

#ifdef __cplusplus
}
#endif

#endif // ADAPTER_CORE_INC_DEVICE_HPP
