#ifndef ADAPTER_MODULES_DCP_CORE_FRAME_HPP
#define ADAPTER_MODULES_DCP_CORE_FRAME_HPP

#include <stdint.h>

#include "dcp/core/message_id.hpp"
#include "dcp/core/constants.hpp"
#include "dcp/core/types.hpp"

namespace dcp {

struct FrameHeader {
  uint8_t version;
  MessageType message_type;
  PeripheralType peripheral;
  uint8_t resource_id;
  MessageId command;
  uint32_t request_id;
  uint16_t payload_length;
};

struct Frame {
  FrameHeader header;
  uint8_t payload[kMaxPayloadSize] {};
};

} // namespace dcp

#endif // ADAPTER_MODULES_DCP_CORE_FRAME_HPP
