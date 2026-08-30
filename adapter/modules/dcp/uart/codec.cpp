#include "dcp/uart/codec.hpp"

#include "dcp/internal/byte_reader.hpp"
#include "dcp/internal/byte_writer.hpp"

namespace dcp::uart {

DecodeResult Codec::DecodeConfigRequest(const uint8_t *payload, uint16_t length, Config &config) {
  internal::ByteReader reader(payload, length);

  if (!reader.ReadU32(config.baud_rate)) return DecodeResult::kInvalidLength;
  if (!reader.ReadU8(config.data_bits)) return DecodeResult::kInvalidLength;

  uint8_t parity = 0, stop_bits = 0;
  if (!reader.ReadU8(parity)) return DecodeResult::kInvalidLength;
  if (!reader.ReadU8(stop_bits)) return DecodeResult::kInvalidLength;

  if (parity > static_cast<uint8_t>(Parity::kOdd)) return DecodeResult::kInvalidValue;
  if (stop_bits > static_cast<uint8_t>(StopBits::kTwo)) return DecodeResult::kInvalidValue;

  config.parity = static_cast<Parity>(parity);
  config.stop_bits = static_cast<StopBits>(stop_bits);

  if (reader.Remaining() != 0) return DecodeResult::kInvalidLength;
  return DecodeResult::kOk;
}

DecodeResult Codec::DecodeWriteRequest(const uint8_t *payload, uint16_t length, WriteRequest &request) {
  internal::ByteReader reader(payload, length);

  if (!reader.ReadU32(request.execute_after_us)) return DecodeResult::kInvalidLength;
  if (!reader.ReadU16(request.data_length)) return DecodeResult::kInvalidLength;
  if (request.data_length > kMaxDataSize) return DecodeResult::kInvalidValue;
  if (!reader.ReadBytes(request.data, request.data_length)) return DecodeResult::kInvalidLength;

  if (reader.Remaining() != 0) return DecodeResult::kInvalidLength;
  return DecodeResult::kOk;
}

EncodeResult Codec::EncodeConfig(const Config &config, uint8_t *dst, uint16_t capacity) {
  internal::ByteWriter writer(dst, capacity);
  if (!writer.WriteU32(config.baud_rate)) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};
  if (!writer.WriteU8(config.data_bits)) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};
  if (!writer.WriteU8(static_cast<uint8_t>(config.parity))) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};
  if (!writer.WriteU8(static_cast<uint8_t>(config.stop_bits))) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};
  return EncodeResult{EncodeStatus::kOk, writer.Size()};
}

EncodeResult Codec::EncodeWriteResponse(const WriteResponse &response, uint8_t *dst, uint16_t capacity) {
  internal::ByteWriter writer(dst, capacity);
  if (!writer.WriteU16(response.bytes_accepted)) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};
  return EncodeResult{EncodeStatus::kOk, writer.Size()};
}

EncodeResult Codec::EncodeRxEvent(const RxEvent &event, uint8_t *dst, uint16_t capacity) {
  internal::ByteWriter writer(dst, capacity);
  if (!writer.WriteU32(event.timestamp_us)) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};
  if (!writer.WriteU16(event.data_length)) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};
  if (!writer.WriteBytes(event.data, event.data_length)) return EncodeResult{EncodeStatus::kBufferTooSmall, 0};
  return EncodeResult{EncodeStatus::kOk, writer.Size()};
}

} // namespace dcp::uart
