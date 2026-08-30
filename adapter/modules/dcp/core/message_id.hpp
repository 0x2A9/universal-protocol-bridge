#ifndef ADAPTER_MODULES_DCP_CORE_MESSAGE_ID_HPP
#define ADAPTER_MODULES_DCP_CORE_MESSAGE_ID_HPP

#include <stdint.h>

namespace dcp {

enum class MessageId : uint16_t {
  /* System: 0x0000 - 0x00FF */
  kGetProtocolInfo = 0x0001, /* reserved info */
  kPing = 0x0002, /* reserved for PING */
  kGetCapabilities = 0x0003,
  kGetStatus = 0x0004, /* reserved */

  /* Common peripheral: 0x0100 - 0x01FF */
  kConfigure = 0x0100,
  kGetConfig = 0x0101,
  kEnable = 0x0102, /* reserved */
  kDisable = 0x0103, /* reserved */
  kReset = 0x0104,

  /* UART: 0x0200 - 0x02FF */
  kWriteUart = 0x0200,

  /* I2C: 0x0300 - 0x03FF (reserved, not implemented) */

  /* Events */
  kUartRxEvent = 0x8000,
  kHeartbeatEvent = 0x8001, /* reserved */
  kErrorEvent = 0x8F00,
  kOverflowEvent = 0x8F01, /* reserved */
};

} // namespace dcp

#endif // ADAPTER_MODULES_DCP_CORE_MESSAGE_ID_HPP
