#ifndef ADAPTER_MODULES_DCP_INTERNAL_CRC16_HPP
#define ADAPTER_MODULES_DCP_INTERNAL_CRC16_HPP

#include <stdint.h>

namespace dcp::internal {

/* CRC-16/CCITT-FALSE: poly 0x1021, init 0xFFFF, no reflect, no xorout.
 * `seed` lets a caller chain the CRC across non-contiguous buffers (e.g.
 * header bytes then payload bytes) by passing the previous call's result
 * back in as the next call's seed. */
uint16_t Crc16Ccitt(const uint8_t *data, uint16_t len, uint16_t seed = 0xFFFF);

} // namespace dcp::internal

#endif // ADAPTER_MODULES_DCP_INTERNAL_CRC16_HPP
