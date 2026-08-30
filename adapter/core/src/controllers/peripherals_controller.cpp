#include "controllers/peripherals_controller.hpp"

#include "board.h"

/* ------------------------------------------------------------------- */
/* PeripheralsController -- board-specific orchestration, wraps Uart.   */
/* No HAL calls appear anywhere else in the app layer.                 */
/* ------------------------------------------------------------------- */

bool PeripheralsController::Init(void) {
  return uart_.Init();
}

void PeripheralsController::ConfigureUart(const dcp::uart::Config &cfg) {
  uart_cfg_.baud = cfg.baud_rate;
  uart_cfg_.data_bits = cfg.data_bits;
  uart_cfg_.stop_bits = (cfg.stop_bits == dcp::uart::StopBits::kTwo) ? 2 : 1;
  uart_cfg_.parity = static_cast<uint8_t>(cfg.parity); /* Parity/StopBits numbering
                                                          * matches UartConfig's
                                                          * 0=None,1=Even,2=Odd */

  uart_.Reconfigure(uart_cfg_);
}

dcp::uart::Config PeripheralsController::GetUartConfig(void) const {
  dcp::uart::Config cfg{};
  cfg.baud_rate = uart_cfg_.baud;
  cfg.data_bits = uart_cfg_.data_bits;
  cfg.parity = static_cast<dcp::uart::Parity>(uart_cfg_.parity);
  cfg.stop_bits = (uart_cfg_.stop_bits >= 2) ? dcp::uart::StopBits::kTwo : dcp::uart::StopBits::kOne;

  return cfg;
}

void PeripheralsController::ResetUart(void) {
  uart_.Reconfigure(uart_cfg_);
}

uint16_t PeripheralsController::WriteUart(const uint8_t *data, uint16_t len) {
  const uint8_t status = uart_.Transmit(const_cast<uint8_t *>(data), len);
  return (status == HAL_OK) ? len : 0;
}

uint16_t PeripheralsController::ReadUart(uint8_t *dst, uint16_t cap) {
  if (!uart_.IsNewRxData()) return 0;

  uint16_t n = uart_.DequeueRx(dst, cap);
  uart_.ClearNewRxDataFlag();

  return n;
}

void PeripheralsController::ResetSystem(void) {
  NVIC_SystemReset();
}
