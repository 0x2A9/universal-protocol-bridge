#ifndef ADAPTER_MODULES_DCP_CORE_FRAME_ENCODER_HPP
#define ADAPTER_MODULES_DCP_CORE_FRAME_ENCODER_HPP

#include <stdint.h>

#include "dcp/core/frame.hpp"
#include "dcp/core/result.hpp"

namespace dcp {

class FrameEncoder {
 public:
  FrameEncoder() = delete;

  static EncodeResult Encode(const FrameHeader &header, const uint8_t *payload,
                              uint16_t payload_length, uint8_t *dst, uint16_t capacity);
};

} // namespace dcp

#endif // ADAPTER_MODULES_DCP_CORE_FRAME_ENCODER_HPP
