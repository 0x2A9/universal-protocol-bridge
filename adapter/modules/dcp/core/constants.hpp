#ifndef ADAPTER_MODULES_DCP_CORE_CONSTANTS_HPP
#define ADAPTER_MODULES_DCP_CORE_CONSTANTS_HPP

#include <stdint.h>

namespace dcp {

constexpr uint8_t kSyncByte1 = 0xA5;
constexpr uint8_t kSyncByte2 = 0x5A;
constexpr uint8_t kProtocolVersion = 1;
constexpr uint16_t kMaxPayloadSize = 256;
constexpr uint16_t kHeaderSize = 14; /* includes the two sync bytes */
constexpr uint16_t kMaxFrameSize = kHeaderSize + kMaxPayloadSize + 2 /* crc16 */;

} // namespace dcp

#endif // ADAPTER_MODULES_DCP_CORE_CONSTANTS_HPP
