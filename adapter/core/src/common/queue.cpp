#include "queue.hpp"

uint16_t Queue::Count() const {
  return (head_ - tail_) & kMask;
}

uint16_t Queue::Free() const {
  return kMask - Count(); // for one slot empty queue
}

uint16_t Queue::Pop(uint8_t* dst, uint32_t len) {
  uint16_t n = Count();
  if (n > len) n = len;

  for (uint32_t i = 0; i < n; i++) {
    dst[i] = buf_[tail_];
    tail_ = (tail_ + 1) & kMask;
  }
  return n;
}

void Queue::Push(const uint8_t* data, uint32_t len) {
  for (uint32_t i = 0; i < len; i++) {
    uint16_t next = (head_ + 1) & kMask;
    if (next == tail_) break;

    buf_[head_] = data[i];
    head_ = next;
  }
}
