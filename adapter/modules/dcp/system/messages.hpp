#ifndef ADAPTER_MODULES_DCP_SYSTEM_MESSAGES_HPP
#define ADAPTER_MODULES_DCP_SYSTEM_MESSAGES_HPP

#include <stdint.h>

namespace dcp::system {

struct Capabilities {
  uint8_t fw_version_major;
  uint8_t fw_version_minor;
  uint8_t fw_version_patch;
  uint32_t supported_peripherals_mask;
};

} // namespace dcp::system

#endif // ADAPTER_MODULES_DCP_SYSTEM_MESSAGES_HPP
