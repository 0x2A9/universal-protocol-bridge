#ifndef ADAPTER_MODULES_DCP_EVENT_HPP
#define ADAPTER_MODULES_DCP_EVENT_HPP

#include <stdint.h>

#include "dcp/core/result.hpp"
#include "dcp/request.hpp"
#include "dcp/uart/messages.hpp"

namespace dcp {

enum class EventPayloadType : uint8_t {
  kNone,
  kUartRxEvent,
  kError,
};

struct Event {
  MessageHeader header;
  EventPayloadType payload_type;
  union {
    uart::RxEvent uart_rx;
    Status error_status;
  } payload;
};

} // namespace dcp

#endif // ADAPTER_MODULES_DCP_EVENT_HPP
