#ifndef ADAPTER_MODULES_DCP_INTERNAL_BYTE_READER_HPP
#define ADAPTER_MODULES_DCP_INTERNAL_BYTE_READER_HPP

#include <stdint.h>

namespace dcp::internal {

class ByteReader {
 public:
  ByteReader(const uint8_t *data, uint16_t size) : data_(data), size_(size), pos_(0) {}

  bool ReadU8(uint8_t &value);
  bool ReadU16(uint16_t &value);
  bool ReadU32(uint32_t &value);
  bool ReadBytes(uint8_t *dst, uint16_t length);

  uint16_t Remaining(void) const { return size_ - pos_; }

 private:
  const uint8_t *data_;
  uint16_t size_;
  uint16_t pos_;
};

} // namespace dcp::internal

#endif // ADAPTER_MODULES_DCP_INTERNAL_BYTE_READER_HPP
