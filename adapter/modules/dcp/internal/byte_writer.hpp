#ifndef ADAPTER_MODULES_DCP_INTERNAL_BYTE_WRITER_HPP
#define ADAPTER_MODULES_DCP_INTERNAL_BYTE_WRITER_HPP

#include <stdint.h>

namespace dcp::internal {

class ByteWriter {
 public:
  ByteWriter(uint8_t *data, uint16_t capacity) : data_(data), capacity_(capacity), pos_(0) {}

  bool WriteU8(uint8_t value);
  bool WriteU16(uint16_t value);
  bool WriteU32(uint32_t value);
  bool WriteBytes(const uint8_t *data, uint16_t len);

  uint16_t Size(void) const { return pos_; }

 private:
  uint8_t *data_;
  uint16_t capacity_;
  uint16_t pos_;
};

} // namespace dcp::internal

#endif // ADAPTER_MODULES_DCP_INTERNAL_BYTE_WRITER_HPP
