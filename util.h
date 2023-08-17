#pragma once

#include <stdint.h>
#include <stdlib.h>

inline size_t volatile_memcpy(volatile void *dst, const volatile void *src,
                              size_t n) {
  auto *d = reinterpret_cast<volatile uint8_t *>(dst);
  auto *s = reinterpret_cast<const volatile uint8_t *>(src);
  for (size_t i = 0; i < n; ++i)
    d[i] = s[i];
  return n;
}