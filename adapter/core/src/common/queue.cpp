#include "queue.hpp"

uint16_t Queue::Count() const {
  if (head_ == tail_) {
    return full_ ? kSize : 0;
  }
  return (head_ - tail_) & kMask;
}

uint16_t Queue::Free() const {
  return (uint16_t)(kSize - Count());
}

uint16_t Queue::Pop(uint8_t *dst, uint32_t len) {
  uint16_t n = Count();
  if (n > len) n = (uint16_t)len;

  for (uint16_t i = 0; i < n; i++) {
    dst[i] = buf_[tail_];
    tail_ = (tail_ + 1) & kMask;
    full_ = false;
  }
  return n;
}

void Queue::Push(const uint8_t *data, uint32_t len) {
  for (uint32_t i = 0; i < len; i++) {
    if (full_) break; 

    buf_[head_] = data[i];
    head_ = (head_ + 1) & kMask;

    if (head_ == tail_) full_ = true;
  }
}
