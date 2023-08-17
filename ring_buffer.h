#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <util/atomic.h>

#include "util.h"

namespace kvmd {

template <typename T, typename size_type, size_type size_val>
struct ring_buffer {
  constexpr static size_type size = size_val;

  bool push(const T *val = nullptr) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      size_type write_slot = next_slot(write_index_);
      if (write_slot == read_index_)
        next(nullptr);
      if (val)
        volatile_memcpy(&buffer_[write_index_], val, sizeof(T));
      write_index_ = write_slot;
    }
    return true;
  }

  bool next(T *val = nullptr) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      if (empty())
        return false;
      if (val)
        volatile_memcpy(val, &buffer_[read_index_], sizeof(T));
      read_index_ = next_slot(read_index_);
    }
    return true;
  }

  inline bool empty() { return read_index_ == write_index_; }

private:
  inline size_type next_slot(size_type idx) { return (idx + 1) % size; }

  volatile T buffer_[size];
  volatile size_type read_index_ = 0;
  volatile size_type write_index_ = 0;
};

} // namespace kvmd