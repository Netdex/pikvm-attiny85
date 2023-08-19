#define LAYOUT_US_ENGLISH
#include <TrinketHidCombo.h>

#include "keymap.h"
#include "protocol.h"
#include "spi.h"
#include "state.h"

namespace {
keyboard kbd;
uint32_t last_usb_poll_ms = 0;

uint8_t handle_keyboard_key(const kvmd::args &data) {
  uint8_t code = data.key.code;
  bool state = data.key.state;
  kbd(code, state);
  TrinketHidCombo.pressKeys(kbd.modifier(), kbd.scancodes(),
                            keyboard::key_limit);
  return kvmd::PONG_OK;
}

uint8_t handle_mouse_button(const kvmd::args &data) {
  // uint8_t state = ;
  // uint8_t extra = msg[1];

  // if (state & kvmd::COMMAND_MOUSE_LEFT_SELECT) {
  //   bool left = state & kvmd::COMMAND_MOUSE_LEFT_STATE;
  // }
  // if (state & kvmd::COMMAND_MOUSE_RIGHT_SELECT) {
  //   bool right = state & kvmd::COMMAND_MOUSE_RIGHT_STATE;
  // }
  return kvmd::PONG_OK;
}

uint8_t handle_request(const kvmd::message &msg) {
  if (kvmd::crc16(msg.bytes, 6) == kvmd::swap16(msg.crc)) {
    switch (msg.op) {
    case kvmd::COMMAND_KEYBOARD_KEY:
      return handle_keyboard_key(msg.data);
    case kvmd::COMMAND_MOUSE_BUTTON:
      return handle_mouse_button(msg.data);
    case kvmd::COMMAND_PING:
      return kvmd::PONG_OK;
    case kvmd::COMMAND_REPEAT:
      return 0;
    case kvmd::COMMAND_SET_KEYBOARD:
    case kvmd::COMMAND_SET_MOUSE:
    case kvmd::COMMAND_SET_CONNECTED:
    case kvmd::COMMAND_CLEAR_HID:
    case kvmd::COMMAND_MOUSE_MOVE:
    case kvmd::COMMAND_MOUSE_RELATIVE:
    case kvmd::COMMAND_MOUSE_WHEEL:
    default:
      return kvmd::RESPONSE_INVALID_ERROR;
    }
  }
  return kvmd::RESPONSE_CRC_ERROR;
}

void send_response(uint8_t code) {
  static uint8_t last_code = kvmd::RESPONSE_NONE;
  code = code ? code : last_code; // repeat the last code
  kvmd::message response = {0};
  response.magic = kvmd::MAGIC_RESP;
  if (code & kvmd::PONG_OK) {
    response.op = kvmd::PONG_OK;
    response.data.resp.output1 |= kvmd::OUTPUTS1_KEYBOARD_USB;
    // response.data.resp.output1 |= kvmd::OUTPUTS1_MOUSE_USB_REL;
    response.data.resp.output2 |= kvmd::OUTPUTS2_CONNECTED;
    response.data.resp.output2 |= kvmd::OUTPUTS2_HAS_USB;
  } else {
    response.op = code;
  }
  response.crc = kvmd::swap16(kvmd::crc16(response.bytes, 6));
  spi_tx_write(&response);
}
} // namespace

void setup() {
  spi_usi_init();
  TrinketHidCombo.begin();
}

void loop() {
  kvmd::message msg;
  if (spi_rx_get(&msg)) {
    send_response(handle_request(msg));
  }
  auto now = millis();
  if (now - last_usb_poll_ms >= 10) {
    TrinketHidCombo.poll();
    last_usb_poll_ms = now;
  }
}
