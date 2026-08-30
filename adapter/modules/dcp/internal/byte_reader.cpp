#include "dcp/internal/byte_reader.hpp"

namespace dcp::internal {

bool ByteReader::ReadU8(uint8_t &value) {
  if (Remaining() < 1) return false;
  value = data_[pos_];
  pos_ += 1;
  return true;
}

bool ByteReader::ReadU16(uint16_t &value) {
  if (Remaining() < 2) return false;
  value = static_cast<uint16_t>(data_[pos_]) |
          (static_cast<uint16_t>(data_[pos_ + 1]) << 8);
  pos_ += 2;
  return true;
}

bool ByteReader::ReadU32(uint32_t &value) {
  if (Remaining() < 4) return false;
  value = static_cast<uint32_t>(data_[pos_]) |
          (static_cast<uint32_t>(data_[pos_ + 1]) << 8) |
          (static_cast<uint32_t>(data_[pos_ + 2]) << 16) |
          (static_cast<uint32_t>(data_[pos_ + 3]) << 24);
  pos_ += 4;
  return true;
}

bool ByteReader::ReadBytes(uint8_t *dst, uint16_t length) {
  if (Remaining() < length) return false;
  for (uint16_t i = 0; i < length; i++) dst[i] = data_[pos_ + i];
  pos_ += length;
  return true;
}

} // namespace dcp::internal
