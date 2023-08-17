#pragma once

#include <stdint.h>
#include <string.h>

#include "ring_buffer.h"
#include "util.h"


namespace kvmd {

union __attribute__((packed)) args {
  struct {
    uint8_t code;
    bool state;
  } key;
  struct {
    uint8_t output1;
    uint8_t output2;
  } resp;
  uint8_t bytes[4];
};

// msg[  0]: magic
// msg[  1]: op
// msg[2:5]: data
// msg[6:7]: crc
union __attribute__((packed)) message {
  struct {
    uint8_t magic;
    uint8_t op;
    args data;
    uint16_t crc;
  };
  uint8_t bytes[8];
};

using message_buffer = ring_buffer<message, uint8_t, 4>;

} // namespace kvmd