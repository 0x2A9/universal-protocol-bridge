#ifndef ADAPTER_MODULES_DCP_RESPONSE_HPP
#define ADAPTER_MODULES_DCP_RESPONSE_HPP

#include <stdint.h>

#include "dcp/core/result.hpp"
#include "dcp/request.hpp"
#include "dcp/system/messages.hpp"
#include "dcp/uart/messages.hpp"

namespace dcp {

enum class ResponsePayloadType : uint8_t {
  kNone,
  kCapabilities,
  kUartConfig,
  kUartWrite,
};

struct Response {
  MessageHeader header;
  Status status;
  ResponsePayloadType payload_type;
  union {
    system::Capabilities capabilities;
    uart::Config uart_config;
    uart::WriteResponse uart_write;
  } payload;
};

} // namespace dcp

#endif // ADAPTER_MODULES_DCP_RESPONSE_HPP
