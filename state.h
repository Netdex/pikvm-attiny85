#pragma once

#include "keymap.h"
#include <stdint.h>
#include <string.h>

// state machine converting commands to hid reports
struct keyboard {
  constexpr static size_t key_limit = 5;

  keyboard() { reset(); }

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
        reset();
      }
    }
  }

  uint8_t modifier() { return modifier_; }
  uint8_t *scancodes() { return scancodes_; }

  void reset() {
    modifier_ = 0;
    memset(scancodes_, 0, key_limit);
  }

private:
  uint8_t modifier_;
  uint8_t scancodes_[key_limit];
};

enum class mouse_button : uint8_t { LEFT = 0x1, RIGHT = 0x2, MIDDLE = 0x4 };

struct mouse {
  mouse() { reset(); }

  void operator()(mouse_button btn, bool select, bool state) {
    if (select) {
      switch (btn) {
      case mouse_button::LEFT:
        left = state;
        break;
      case mouse_button::RIGHT:
        right = state;
        break;
      case mouse_button::MIDDLE:
        middle = state;
        break;
      }
    }
  }

  uint8_t button() const {
    return (left ? uint8_t(mouse_button::LEFT) : 0) |
           (right ? uint8_t(mouse_button::RIGHT) : 0) |
           (middle ? uint8_t(mouse_button::MIDDLE) : 0);
  }

  void reset() {
    left = false;
    right = false;
    middle = false;
  }

  bool left;
  bool right;
  bool middle;
};