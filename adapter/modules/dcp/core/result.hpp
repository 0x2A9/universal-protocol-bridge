#ifndef ADAPTER_MODULES_DCP_CORE_RESULT_HPP
#define ADAPTER_MODULES_DCP_CORE_RESULT_HPP

#include <stdint.h>

namespace dcp {

enum class DecodeResult : uint8_t {
  kOk,
  kInvalidLength,
  kInvalidValue,
  kUnsupported,
};

enum class EncodeStatus : uint8_t {
  kOk,
  kBufferTooSmall,
  kInvalidValue,
};

struct EncodeResult {
  EncodeStatus status;
  uint16_t size;
};

enum class ParseResult : uint8_t {
  kOk,
  kNoFrame,
  kInvalidCrc,
  kInvalidVersion,
  kFrameTooLarge,
  kUnexpectedMessageType,
};

enum class Status : uint16_t {
  kOk = 0,
  kInvalidCommand,
  kInvalidParameter,
  kInvalidState,
  kNotSupported,
  kBusy,
  kTimeout,
  kBufferFull,
  kNack, /* reserved for I2C */
  kHardwareError,
  kProtocolError,
};

} // namespace dcp

#endif // ADAPTER_MODULES_DCP_CORE_RESULT_HPP
