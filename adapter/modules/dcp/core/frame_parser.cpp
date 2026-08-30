#include "dcp/core/frame_parser.hpp"

#include "dcp/core/constants.hpp"
#include "dcp/internal/byte_reader.hpp"
#include "dcp/internal/crc16.hpp"

namespace dcp {

FrameParser::FrameParser(MessageType accepted_type) : accepted_type_(accepted_type) {}

void FrameParser::Reset(void) {
  state_ = State::kSync1;
  header_pos_ = 0;
  payload_pos_ = 0;
  crc_pos_ = 0;
  frame_ready_ = false;
}

ParseResult FrameParser::ParseByte(uint8_t b) {
  switch (state_) {
    case State::kSync1:
      if (b == kSyncByte1) state_ = State::kSync2;
      return ParseResult::kNoFrame;

    case State::kSync2:
      if (b == kSyncByte2) {
        state_ = State::kHeader;
        header_pos_ = 0;
      } else if (b != kSyncByte1) {
        state_ = State::kSync1;
      }
      return ParseResult::kNoFrame;

    case State::kHeader: {
      header_buf_[header_pos_++] = b;
      if (header_pos_ < sizeof(header_buf_)) return ParseResult::kNoFrame;

      internal::ByteReader reader(header_buf_, sizeof(header_buf_));
      uint8_t version = 0, message_type = 0, peripheral = 0, resource_id = 0;
      uint16_t command = 0, payload_length = 0;
      uint32_t request_id = 0;
      reader.ReadU8(version);
      reader.ReadU8(message_type);
      reader.ReadU8(peripheral);
      reader.ReadU8(resource_id);
      reader.ReadU16(command);
      reader.ReadU32(request_id);
      reader.ReadU16(payload_length);

      if (version != kProtocolVersion) {
        state_ = State::kSync1;
        return ParseResult::kInvalidVersion;
      }
      if (static_cast<MessageType>(message_type) != accepted_type_) {
        state_ = State::kSync1;
        return ParseResult::kUnexpectedMessageType;
      }
      if (payload_length > kMaxPayloadSize) {
        state_ = State::kSync1;
        return ParseResult::kFrameTooLarge;
      }

      frame_.header.version = version;
      frame_.header.message_type = static_cast<MessageType>(message_type);
      frame_.header.peripheral = static_cast<PeripheralType>(peripheral);
      frame_.header.resource_id = resource_id;
      frame_.header.command = static_cast<MessageId>(command);
      frame_.header.request_id = request_id;
      frame_.header.payload_length = payload_length;

      payload_pos_ = 0;
      if (payload_length == 0) {
        state_ = State::kCrc;
        crc_pos_ = 0;
      } else {
        state_ = State::kPayload;
      }
      return ParseResult::kNoFrame;
    }

    case State::kPayload:
      frame_.payload[payload_pos_++] = b;
      if (payload_pos_ < frame_.header.payload_length) return ParseResult::kNoFrame;
      state_ = State::kCrc;
      crc_pos_ = 0;
      return ParseResult::kNoFrame;

    case State::kCrc: {
      crc_buf_[crc_pos_++] = b;
      if (crc_pos_ < 2) return ParseResult::kNoFrame;

      const uint16_t received_crc =
          static_cast<uint16_t>(crc_buf_[0]) | (static_cast<uint16_t>(crc_buf_[1]) << 8);

      uint16_t computed_crc = internal::Crc16Ccitt(header_buf_, sizeof(header_buf_));
      computed_crc = internal::Crc16Ccitt(frame_.payload, frame_.header.payload_length, computed_crc);

      state_ = State::kSync1;

      if (received_crc != computed_crc) return ParseResult::kInvalidCrc;

      frame_ready_ = true;
      return ParseResult::kOk;
    }
  }

  return ParseResult::kNoFrame;
}

ParseResult FrameParser::Parse(const uint8_t *data, uint16_t length) {
  if (length > 0) rx_.Push(data, length);

  ParseResult result = ParseResult::kNoFrame;
  uint8_t b;
  while (!frame_ready_ && rx_.Pop(&b, 1) == 1) {
    result = ParseByte(b);
    if (result != ParseResult::kNoFrame) return result;
  }
  return result;
}

bool FrameParser::HasFrame(void) const { return frame_ready_; }

bool FrameParser::PopFrame(Frame &frame) {
  if (!frame_ready_) return false;
  frame = frame_;
  frame_ready_ = false;
  return true;
}

} // namespace dcp
