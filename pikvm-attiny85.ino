#define LAYOUT_US_ENGLISH
#include <TrinketHidCombo.h>

#include "keymap.h"
#include "protocol.h"
#include "spi.h"
#include "state.h"

static keyboard kbd;

static uint8_t handle_keyboard_key(const uint8_t *msg) {
  uint8_t code = msg[0];
  bool state = msg[1];
  kbd(code, state);
  TrinketHidCombo.pressKeys(kbd.modifier(), kbd.scancodes(),
                            keyboard::key_limit);
  return kvmd::PONG_OK;
}

static uint8_t handle_mouse_button(const uint8_t *msg) {
  uint8_t state = msg[0];
  uint8_t extra = msg[1];

  if (state & kvmd::COMMAND_MOUSE_LEFT_SELECT) {
    bool left = state & kvmd::COMMAND_MOUSE_LEFT_STATE;
  }
  if (state & kvmd::COMMAND_MOUSE_RIGHT_SELECT) {
    bool right = state & kvmd::COMMAND_MOUSE_RIGHT_STATE;
  }
  return kvmd::PONG_OK;
}

static uint8_t handle_request(const uint8_t *data) {
  if (kvmd::crc16(data, 6) == kvmd::from_be16(&data[6])) {
    const uint8_t *msg = data + 2;

    switch (data[1]) {
    case kvmd::COMMAND_PING:
      return kvmd::PONG_OK;
    case kvmd::COMMAND_SET_KEYBOARD:
      return kvmd::PONG_OK;
    case kvmd::COMMAND_SET_MOUSE:
      return kvmd::PONG_OK;
    case kvmd::COMMAND_SET_CONNECTED:
      return kvmd::PONG_OK;
    case kvmd::COMMAND_CLEAR_HID:
      return kvmd::PONG_OK;
    case kvmd::COMMAND_KEYBOARD_KEY:
      return handle_keyboard_key(msg);
    case kvmd::COMMAND_MOUSE_BUTTON:
      return handle_mouse_button(msg);
    case kvmd::COMMAND_MOUSE_MOVE:
      return kvmd::PONG_OK;
    case kvmd::COMMAND_MOUSE_RELATIVE:
      return kvmd::PONG_OK;
    case kvmd::COMMAND_MOUSE_WHEEL:
      return kvmd::PONG_OK;
    case kvmd::COMMAND_REPEAT:
      return 0;
    default:
      return kvmd::RESPONSE_INVALID_ERROR;
    }
  }
  return kvmd::RESPONSE_CRC_ERROR;
}

static void send_response(uint8_t code) {
  static uint8_t last_code = kvmd::RESPONSE_NONE;
  code = code ? code : last_code; // repeat the last code

  uint8_t response[8] = {0};
  response[0] = kvmd::MAGIC_RESP;
  if (code & kvmd::PONG_OK) {
    response[1] = kvmd::PONG_OK;
    response[2] |= kvmd::OUTPUTS1_KEYBOARD_USB;
    response[3] |= kvmd::OUTPUTS2_CONNECTED;
    response[3] |= kvmd::OUTPUTS2_HAS_USB;
  } else {
    response[1] = code;
  }
  kvmd::to_be16(kvmd::crc16(response, 6), &response[6]);

  spi_tx_write(response);
}

void setup() {
  spi_usi_init();
  TrinketHidCombo.begin();
}

void loop() {
  if (spi_rx_ready()) {
    uint8_t data[KVMD_MSG_SZ];
    volatile uint8_t *rx_data = spi_rx_get();
    for (int i = 0; i < KVMD_MSG_SZ; ++i) {
      data[i] = rx_data[i];
    }
    send_response(handle_request(data));
  }
  TrinketHidCombo.poll();
}
