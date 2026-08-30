#ifndef ADAPTER_MODULES_DCP_CORE_TYPES_HPP
#define ADAPTER_MODULES_DCP_CORE_TYPES_HPP

#include <stdint.h>

namespace dcp {

enum class MessageType : uint8_t {
  kRequest = 0x01,
  kResponse = 0x02,
  kEvent = 0x03,
};

enum class PeripheralType : uint8_t {
  kSystem = 0x00,
  kUart = 0x01,
  kI2c = 0x02, /* tag only, no codec/messages yet */
};

} // namespace dcp

#endif // ADAPTER_MODULES_DCP_CORE_TYPES_HPP
