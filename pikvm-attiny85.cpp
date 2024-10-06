// ATtiny85:
// https://www.mouser.com/datasheet/2/268/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATti-1315542.pdf

#include <avr/interrupt.h>

#include <TrinketHidCombo.h>

#include "protocol.h"
#include "spi.h"
#include "state.h"
#include "timer.h"

namespace {
keyboard kbd;
mouse ms;

kvmd::code_t handle_keyboard_key(const kvmd::args &data) {
  uint8_t code = data.key.code;
  bool state = data.key.state;
  kbd(code, state);
  TrinketHidCombo.pressKeys(kbd.modifier(), kbd.scancodes(),
                            keyboard::key_limit);
  return kvmd::PONG_OK;
}

kvmd::code_t handle_mouse_button(const kvmd::args &data) {
  ms(mouse_button::LEFT, data.bytes[0] & kvmd::COMMAND_MOUSE_LEFT_SELECT,
     data.bytes[0] & kvmd::COMMAND_MOUSE_LEFT_STATE);
  ms(mouse_button::RIGHT, data.bytes[0] & kvmd::COMMAND_MOUSE_RIGHT_SELECT,
     data.bytes[0] & kvmd::COMMAND_MOUSE_RIGHT_STATE);
  ms(mouse_button::MIDDLE, data.bytes[0] & kvmd::COMMAND_MOUSE_MIDDLE_SELECT,
     data.bytes[0] & kvmd::COMMAND_MOUSE_MIDDLE_STATE);
  TrinketHidCombo.mouseMove(0, 0, 0, ms.button());
  return kvmd::PONG_OK;
}

kvmd::code_t handle_mouse_move(const kvmd::args &data) {
  int8_t x = int8_t(data.bytes[0]);
  int8_t y = int8_t(data.bytes[1]);
  TrinketHidCombo.mouseMove(x, y, 0, ms.button());
  return kvmd::PONG_OK;
}

kvmd::code_t handle_mouse_wheel(const kvmd::args &data) {
  int8_t delta_y = int8_t(data.bytes[1]);
  TrinketHidCombo.mouseMove(0, 0, delta_y, ms.button());
  return kvmd::PONG_OK;
}

kvmd::code_t handle_request(const kvmd::message &msg) {
  if (kvmd::crc16(msg.bytes, 6) == kvmd::swap16(msg.crc)) {
    switch (msg.op) {
    case kvmd::COMMAND_KEYBOARD_KEY:
      return handle_keyboard_key(msg.data);
    case kvmd::COMMAND_MOUSE_BUTTON:
      return handle_mouse_button(msg.data);
    case kvmd::COMMAND_MOUSE_RELATIVE:
      return handle_mouse_move(msg.data);
    case kvmd::COMMAND_MOUSE_WHEEL:
      return handle_mouse_wheel(msg.data);
    case kvmd::COMMAND_PING:
      return kvmd::PONG_OK;
    case kvmd::COMMAND_REPEAT:
      return 0;
    case kvmd::COMMAND_SET_KEYBOARD:
    case kvmd::COMMAND_SET_MOUSE:
    case kvmd::COMMAND_SET_CONNECTED:
    case kvmd::COMMAND_CLEAR_HID:
    case kvmd::COMMAND_MOUSE_MOVE:
      // TODO: support absolute mouse since it is superior to relative mouse
      return kvmd::PONG_OK;
    default:
      return kvmd::RESPONSE_INVALID_ERROR;
    }
  }
  return kvmd::RESPONSE_CRC_ERROR;
}

void send_response(kvmd::code_t code) {
  static kvmd::code_t last_code = kvmd::RESPONSE_NONE;
  code = code ? code : last_code; // repeat the last code
  kvmd::message response = {{0}};
  response.magic = kvmd::MAGIC_RESP;
  if (code & kvmd::PONG_OK) {
    response.op = kvmd::PONG_OK;
    response.data.resp.output1 |= kvmd::OUTPUTS1_KEYBOARD_USB;
    response.data.resp.output1 |= kvmd::OUTPUTS1_MOUSE_USB_REL;
    response.data.resp.output2 |= kvmd::OUTPUTS2_CONNECTED;
    response.data.resp.output2 |= kvmd::OUTPUTS2_HAS_USB;
  } else {
    response.op = code;
  }
  response.crc = kvmd::swap16(kvmd::crc16(response.bytes, 6));
  spi_tx_write(&response);
}
} // namespace

int main() {
  spi_usi_init();
  timer0_init();
  TrinketHidCombo.begin();

  while (true) {
    kvmd::message msg;
    if (spi_rx_get(&msg)) {
      send_response(handle_request(msg));
    }
  }
  return 0;
}