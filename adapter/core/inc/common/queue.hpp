#ifndef ADAPTER_CORE_INC_COMMON_QUEUE
#define ADAPTER_CORE_INC_COMMON_QUEUE

#include <stdint.h>

class Queue {
 public:
  uint16_t Count() const;
  uint16_t Free() const;
  uint16_t Pop(uint8_t *dst, uint16_t len);
  void Push(const uint8_t *data, uint16_t len);
  uint16_t Peek(uint8_t* dst, uint16_t len) const;
  void Drop(uint16_t len);

 private:
  /* Should be the power of two */
  static constexpr uint16_t kSize = 512;
  static constexpr uint16_t kMask = kSize - 1;

  volatile uint16_t head_ = 0;
  volatile uint16_t tail_ = 0;
  volatile bool full_ = false;

  uint8_t buf_[kSize] {};
};

#endif // ADAPTER_CORE_INC_COMMON_QUEUE
