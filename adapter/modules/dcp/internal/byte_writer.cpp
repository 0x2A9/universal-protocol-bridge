#include "dcp/internal/byte_writer.hpp"

namespace dcp::internal {

bool ByteWriter::WriteU8(uint8_t value) {
  if (pos_ + 1 > capacity_) return false;
  data_[pos_] = value;
  pos_ += 1;
  return true;
}

bool ByteWriter::WriteU16(uint16_t value) {
  if (pos_ + 2 > capacity_) return false;
  data_[pos_] = static_cast<uint8_t>(value & 0xFF);
  data_[pos_ + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
  pos_ += 2;
  return true;
}

bool ByteWriter::WriteU32(uint32_t value) {
  if (pos_ + 4 > capacity_) return false;
  data_[pos_] = static_cast<uint8_t>(value & 0xFF);
  data_[pos_ + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
  data_[pos_ + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
  data_[pos_ + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
  pos_ += 4;
  return true;
}

bool ByteWriter::WriteBytes(const uint8_t *data, uint16_t len) {
  if (pos_ + len > capacity_) return false;
  for (uint16_t i = 0; i < len; i++) data_[pos_ + i] = data[i];
  pos_ += len;
  return true;
}

} // namespace dcp::internal
