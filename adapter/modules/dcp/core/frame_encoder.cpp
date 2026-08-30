#include "dcp/core/frame_encoder.hpp"

#include "dcp/internal/byte_writer.hpp"
#include "dcp/internal/crc16.hpp"

namespace dcp {

EncodeResult FrameEncoder::Encode(const FrameHeader &header, const uint8_t *payload,
                                   uint16_t payload_length, uint8_t *dst, uint16_t capacity) {
  const uint16_t frame_size = kHeaderSize + payload_length + 2;
  if (frame_size > capacity) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};

  internal::ByteWriter writer(dst, capacity);
  writer.WriteU8(kSyncByte1);
  writer.WriteU8(kSyncByte2);

  const uint16_t header_start = writer.Size();
  writer.WriteU8(header.version);
  writer.WriteU8(static_cast<uint8_t>(header.message_type));
  writer.WriteU8(static_cast<uint8_t>(header.peripheral));
  writer.WriteU8(header.resource_id);
  writer.WriteU16(static_cast<uint16_t>(header.command));
  writer.WriteU32(header.request_id);
  writer.WriteU16(payload_length);

  const uint16_t payload_start = writer.Size();
  if (payload_length > 0) writer.WriteBytes(payload, payload_length);

  const uint16_t crc = internal::Crc16Ccitt(
      dst + header_start, static_cast<uint16_t>(payload_start - header_start + payload_length));
  writer.WriteU16(crc);

  return EncodeResult{EncodeStatus::kOk, writer.Size()};
}

} // namespace dcp
