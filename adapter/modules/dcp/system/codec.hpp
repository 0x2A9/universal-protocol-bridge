#ifndef ADAPTER_MODULES_DCP_SYSTEM_CODEC_HPP
#define ADAPTER_MODULES_DCP_SYSTEM_CODEC_HPP

#include <stdint.h>

#include "dcp/core/result.hpp"
#include "dcp/system/messages.hpp"

namespace dcp::system {

class Codec {
 public:
  Codec() = delete;

  static EncodeResult EncodeCapabilitiesResponse(const Capabilities &caps, uint8_t *dst, uint16_t capacity);
  static EncodeResult EncodeErrorEvent(Status status, uint8_t *dst, uint16_t capacity);
};

} // namespace dcp::system

#endif // ADAPTER_MODULES_DCP_SYSTEM_CODEC_HPP
