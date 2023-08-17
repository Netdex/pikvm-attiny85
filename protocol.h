/*****************************************************************************
#                                                                            #
#    KVMD - The main PiKVM daemon.                                           #
#                                                                            #
#    Copyright (C) 2018-2022  Maxim Devaev <mdevaev@gmail.com>               #
#                                                                            #
#    This program is free software: you can redistribute it and/or modify    #
#    it under the terms of the GNU General Public License as published by    #
#    the Free Software Foundation, either version 3 of the License, or       #
#    (at your option) any later version.                                     #
#                                                                            #
#    This program is distributed in the hope that it will be useful,         #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of          #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
#    GNU General Public License for more details.                            #
#                                                                            #
#    You should have received a copy of the GNU General Public License       #
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.  #
#                                                                            #
*****************************************************************************/

#pragma once

#include <stdint.h>

namespace kvmd {
const uint8_t MAGIC = 0x33;
const uint8_t MAGIC_RESP = 0x34;

enum response : uint8_t {
  RESPONSE_NONE = 0x24,
  RESPONSE_CRC_ERROR = 0x40,
  RESPONSE_INVALID_ERROR = 0x45,
  RESPONSE_TIMEOUT_ERROR = 0x48
};

enum pong : uint8_t {
  PONG_OK = 0x80,
  PONG_CAPS = 0b00000001,
  PONG_SCROLL = 0b00000010,
  PONG_NUM = 0b00000100,
  PONG_KEYBOARD_OFFLINE = 0b00001000,
  PONG_MOUSE_OFFLINE = 0b00010000,
  PONG_RESET_REQUIRED = 0b01000000
};

enum outputs1 : uint8_t {
  OUTPUTS1_DYNAMIC = 0b10000000,
  OUTPUTS1_KEYBOARD_MASK = 0b00000111,
  OUTPUTS1_KEYBOARD_USB = 0b00000001,
  OUTPUTS1_KEYBOARD_PS2 = 0b00000011,
  OUTPUTS1_MOUSE_MASK = 0b00111000,
  OUTPUTS1_MOUSE_USB_ABS = 0b00001000,
  OUTPUTS1_MOUSE_USB_REL = 0b00010000,
  OUTPUTS1_MOUSE_PS2 = 0b00011000,
  OUTPUTS1_MOUSE_USB_WIN98 = 0b00100000
};

enum outputs2 : uint8_t {
  OUTPUTS2_CONNECTABLE = 0b10000000,
  OUTPUTS2_CONNECTED = 0b01000000,
  OUTPUTS2_HAS_USB = 0b00000001,
  OUTPUTS2_HAS_PS2 = 0b00000010,
  OUTPUTS2_HAS_USB_WIN98 = 0b00000100,
};

enum command : uint8_t {
  COMMAND_PING = 0x01,
  COMMAND_REPEAT = 0x02,
  COMMAND_SET_KEYBOARD = 0x03,
  COMMAND_SET_MOUSE = 0x04,
  COMMAND_SET_CONNECTED = 0x05,
  COMMAND_CLEAR_HID = 0x10,
  COMMAND_KEYBOARD_KEY = 0x11,
  COMMAND_MOUSE_MOVE = 0x12,
  COMMAND_MOUSE_BUTTON = 0x13,
  COMMAND_MOUSE_WHEEL = 0x14,
  COMMAND_MOUSE_RELATIVE = 0x15,
  COMMAND_MOUSE_LEFT_SELECT = 0b10000000,
  COMMAND_MOUSE_LEFT_STATE = 0b00001000,
  COMMAND_MOUSE_RIGHT_SELECT = 0b01000000,
  COMMAND_MOUSE_RIGHT_STATE = 0b00000100,
  COMMAND_MOUSE_MIDDLE_SELECT = 0b00100000,
  COMMAND_MOUSE_MIDDLE_STATE = 0b00000010,
  COMMAND_MOUSE_EXTRA_UP_SELECT = 0b10000000,
  COMMAND_MOUSE_EXTRA_UP_STATE = 0b00001000,
  COMMAND_MOUSE_EXTRA_DOWN_SELECT = 0b01000000,
  COMMAND_MOUSE_EXTRA_DOWN_STATE = 0b00000100,
};

inline uint16_t crc16(const uint8_t *buffer, unsigned length) {
  const uint16_t polinom = 0xA001;
  uint16_t crc = 0xFFFF;

  for (unsigned byte_count = 0; byte_count < length; ++byte_count) {
    crc = crc ^ buffer[byte_count];
    for (unsigned bit_count = 0; bit_count < 8; ++bit_count) {
      if ((crc & 0x0001) == 0) {
        crc = crc >> 1;
      } else {
        crc = crc >> 1;
        crc = crc ^ polinom;
      }
    }
  }
  return crc;
}

inline uint16_t from_be16(const uint8_t *buf) {
  return (((uint16_t)buf[0] << 8) | (uint16_t)buf[1]);
}
inline uint16_t swap16(uint16_t n) {
  return from_be16(reinterpret_cast<const uint8_t *>(&n));
}

inline void to_be16(uint16_t from, uint8_t *buf) {
  buf[0] = (uint8_t)(from >> 8);
  buf[1] = (uint8_t)(from & 0xFF);
}
}; // namespace kvmd