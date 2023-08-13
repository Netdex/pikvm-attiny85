#pragma once

#include <stdint.h>
#include <string.h>

#include "keymap.h"

struct keyboard {
  constexpr static size_t key_limit = 5;

  void operator()(uint8_t code, bool state) {
    uint8_t scan = keymap_usb(code);
    if (scan >= 0xE0 && scan <= 0xE7) {
      uint8_t mod = 1 << (scan & (0xF));
      if (state) {
        modifier_ |= mod;
      } else {
        modifier_ &= ~mod;
      }
    } else {
      if (state) {
        for (size_t idx = 0; idx < key_limit; ++idx) {
          if (!scancodes_[idx]) {
            scancodes_[idx] = scan;
            return;
          }
        }
        // state full
      } else {
        for (size_t idx = 0; idx < key_limit; ++idx) {
          if (scancodes_[idx] == scan) {
            scancodes_[idx] = 0;
            return;
          }
        }
        // bad state
        memset(scancodes_, 0, key_limit);
      }
    }
  }

  uint8_t modifier() { return modifier_; }
  uint8_t *scancodes() { return scancodes_; }

private:
  uint8_t modifier_ = 0;
  uint8_t scancodes_[key_limit] = {0};
};

struct mouse {
  void operator()(uint8_t code, bool state) {}
};