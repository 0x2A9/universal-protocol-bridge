#ifndef ADAPTER_MODULES_DCP_UART_CODEC_HPP
#define ADAPTER_MODULES_DCP_UART_CODEC_HPP

#include <stdint.h>

#include "dcp/core/result.hpp"
#include "dcp/uart/messages.hpp"

namespace dcp::uart {

class Codec {
 public:
  Codec() = delete;

  static DecodeResult DecodeConfigRequest(const uint8_t *payload, uint16_t length, Config &config);
  static DecodeResult DecodeWriteRequest(const uint8_t *payload, uint16_t length, WriteRequest &request);

  static EncodeResult EncodeConfig(const Config &config, uint8_t *dst, uint16_t capacity);
  static EncodeResult EncodeWriteResponse(const WriteResponse &response, uint8_t *dst, uint16_t capacity);
  static EncodeResult EncodeRxEvent(const RxEvent &event, uint8_t *dst, uint16_t capacity);
};

} // namespace dcp::uart

#endif // ADAPTER_MODULES_DCP_UART_CODEC_HPP
