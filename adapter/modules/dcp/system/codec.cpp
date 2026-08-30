#include "dcp/system/codec.hpp"

#include "dcp/internal/byte_writer.hpp"

namespace dcp::system {

EncodeResult Codec::EncodeCapabilitiesResponse(const Capabilities &caps, uint8_t *dst, uint16_t capacity) {
  internal::ByteWriter writer(dst, capacity);
  if (!writer.WriteU8(caps.fw_version_major)) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};
  if (!writer.WriteU8(caps.fw_version_minor)) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};
  if (!writer.WriteU8(caps.fw_version_patch)) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};
  if (!writer.WriteU32(caps.supported_peripherals_mask)) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};
  return EncodeResult{EncodeStatus::kOk, writer.Size()};
}

EncodeResult Codec::EncodeErrorEvent(Status status, uint8_t *dst, uint16_t capacity) {
  internal::ByteWriter writer(dst, capacity);
  if (!writer.WriteU16(static_cast<uint16_t>(status))) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};
  return EncodeResult{EncodeStatus::kOk, writer.Size()};
}

} // namespace dcp::system
