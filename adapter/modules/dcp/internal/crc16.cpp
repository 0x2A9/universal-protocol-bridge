#include "dcp/internal/crc16.hpp"

namespace dcp::internal {

uint16_t Crc16Ccitt(const uint8_t *data, uint16_t len, uint16_t seed) {
  uint16_t crc = seed;

  for (uint16_t i = 0; i < len; i++) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (uint8_t bit = 0; bit < 8; bit++) {
      crc = (crc & 0x8000) ? static_cast<uint16_t>((crc << 1) ^ 0x1021) : static_cast<uint16_t>(crc << 1);
    }
  }

  return crc;
}

} // namespace dcp::internal
