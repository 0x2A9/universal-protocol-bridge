#ifndef ADAPTER_MODULES_DCP_UART_MESSAGES_HPP
#define ADAPTER_MODULES_DCP_UART_MESSAGES_HPP

#include <stdint.h>

namespace dcp::uart {

constexpr uint16_t kMaxDataSize = 256;

enum class Parity : uint8_t { kNone, kEven, kOdd };
enum class StopBits : uint8_t { kOne, kTwo };

struct Config {
  uint32_t baud_rate;
  uint8_t data_bits;
  Parity parity;
  StopBits stop_bits;
};

struct WriteRequest {
  uint32_t execute_after_us;
  uint16_t data_length;
  uint8_t data[kMaxDataSize];
};

struct WriteResponse {
  uint16_t bytes_accepted;
};

struct RxEvent {
  uint32_t timestamp_us;
  uint16_t data_length;
  uint8_t data[kMaxDataSize];
};

} // namespace dcp::uart

#endif // ADAPTER_MODULES_DCP_UART_MESSAGES_HPP
