#ifndef ADAPTER_MODULES_DCP_REQUEST_HPP
#define ADAPTER_MODULES_DCP_REQUEST_HPP

#include <stdint.h>

#include "dcp/core/message_id.hpp"
#include "dcp/core/types.hpp"
#include "dcp/uart/messages.hpp"

namespace dcp {

struct MessageHeader {
  MessageType message_type;
  PeripheralType peripheral;
  uint8_t resource_id;
  MessageId command;
  uint32_t request_id;
};

enum class RequestPayloadType : uint8_t {
  kNone,
  kUartConfig,
  kUartWrite,
};

struct Request {
  MessageHeader header;
  RequestPayloadType payload_type;
  union {
    uart::Config uart_config;
    uart::WriteRequest uart_write;
  } payload;
};

} // namespace dcp

#endif // ADAPTER_MODULES_DCP_REQUEST_HPP
