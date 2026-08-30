#ifndef ADAPTER_MODULES_DCP_CORE_FRAME_PARSER_HPP
#define ADAPTER_MODULES_DCP_CORE_FRAME_PARSER_HPP

#include <stdint.h>

#include "dcp/core/frame.hpp"
#include "dcp/core/result.hpp"
#include "queue.hpp"

namespace dcp {

class FrameParser {
 public:
  /* `accepted_type` is which MessageType this endpoint is allowed to receive
   * (a device only ever receives Requests) -- validated as soon as the
   * message-type byte is read, so the app never has to re-check it. */
  explicit FrameParser(MessageType accepted_type);

  void Reset(void);

  /* Pushes `data`/`length` (if any) into the internal byte queue, then
   * advances the state machine one byte at a time until either a frame
   * becomes ready, a protocol error is detected, or the queue is drained.
   * Call with data=nullptr, length=0 to keep draining an already-buffered
   * queue (e.g. to pop a second frame that arrived in the same USB chunk)
   * without waiting for new bytes. */
  ParseResult Parse(const uint8_t *data, uint16_t length);

  bool HasFrame(void) const;
  bool PopFrame(Frame &frame);

 private:
  enum class State : uint8_t {
    kSync1,
    kSync2,
    kHeader,
    kPayload,
    kCrc,
  };

  ParseResult ParseByte(uint8_t b);

  MessageType accepted_type_;
  Queue rx_;
  State state_ = State::kSync1;
  Frame frame_;
  uint16_t header_pos_ = 0;
  uint16_t payload_pos_ = 0;
  uint16_t crc_pos_ = 0;
  uint8_t header_buf_[kHeaderSize - 2] {}; /* header fields after the two sync bytes */
  uint8_t crc_buf_[2] {};
  bool frame_ready_ = false;
};

} // namespace dcp

#endif // ADAPTER_MODULES_DCP_CORE_FRAME_PARSER_HPP
