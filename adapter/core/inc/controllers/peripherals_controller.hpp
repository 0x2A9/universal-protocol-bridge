#ifndef ADAPTER_CORE_INC_CONTROLLERS_PERIPHERALS_CONTROLLER_HPP
#define ADAPTER_CORE_INC_CONTROLLERS_PERIPHERALS_CONTROLLER_HPP

#include <stdint.h>

#include "device.hpp"
#include "dcp/uart/messages.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/* Board-specific orchestration wrapping Uart -- no HAL calls appear anywhere
 * else in the app layer. Translates between dcp::uart::Config (the wire
 * format's shape) and UartConfig (the HAL-facing shape); Uart itself needs
 * no changes. */
class PeripheralsController {
 public:
  explicit PeripheralsController(Uart &uart) : uart_(uart) {}

  bool Init(void);
  void ConfigureUart(const dcp::uart::Config &cfg);
  dcp::uart::Config GetUartConfig(void) const;
  void ResetUart(void);
  uint16_t WriteUart(const uint8_t *data, uint16_t len);
  uint16_t ReadUart(uint8_t *dst, uint16_t cap);
  void ResetSystem(void);

 private:
  Uart &uart_;
  UartConfig uart_cfg_;
};

#ifdef __cplusplus
}
#endif

#endif // ADAPTER_CORE_INC_CONTROLLERS_PERIPHERALS_CONTROLLER_HPP
