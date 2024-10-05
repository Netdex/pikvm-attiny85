// ATtiny85:
// https://www.mouser.com/datasheet/2/268/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATti-1315542.pdf
// USI SPI:
// https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ApplicationNotes/ApplicationNotes/Atmel-2582-Using-the-USI-Module-for-SPI-Communication-on-tinyAVR-and-megaAVR-Devices_ApplicationNote_AVR319.pdf

#include "spi.h"

#include <avr/interrupt.h>
#include <util/atomic.h>
#include <wiring.h>

#include "message.h"
#include "protocol.h"

namespace {
volatile bool rx_rdy = false;
volatile bool sync = true;
volatile uint8_t tx_data[KVMD_MSG_SZ] = {0};
volatile uint8_t rx_data[KVMD_MSG_SZ] = {0};
volatile uint8_t rx_cnt = 0;
volatile uint8_t tx_cnt = 0;
} // namespace

void spi_usi_init() {
  cli();               // Deactivate interrupts
  DDRB &= ~(1 << DI);  // Set DI as input
  DDRB &= ~(1 << SCK); // Set SCK as input
  DDRB |= 1 << DO;     // MISO Pin has to be an output

  // PB3 and PB4 are used for V-USB

  // Pull reset to ground (causes interference issues)
  DDRB |= 1 << PB5;
  PORTB &= ~(1 << PB5);

  // Activate 3- Wire Mode and use of external clock and enable overflow
  // interrupt
  USICR = ((1 << USIWM0) | (1 << USICS1) | (1 << USIOIE));

  delay(500); // Let things settle
  sei();      // Activate interrupts
}

bool spi_rx_get(kvmd::message *msg) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (!tx_data[0] && rx_cnt == KVMD_MSG_SZ) {
      volatile_memcpy(msg, rx_data, KVMD_MSG_SZ);
      return true;
    }
  }
  return false;
}

void spi_tx_write(const kvmd::message *msg) {
  for (int i = KVMD_MSG_SZ - 1; i >= 0; --i) {
    tx_data[i] = msg->bytes[i];
  }
}

// USI interrupt routine. Always executed when 4-bit overflows (after 16 clock
// edges = 8 clock cycles):
ISR(USI_OVF_vect) {
  USISR = 1 << USIOIF;
  if (tx_data[0] && tx_cnt < KVMD_MSG_SZ) {
    // transmit data
    USIDR = tx_data[tx_cnt];
    ++tx_cnt;
    if (tx_cnt == KVMD_MSG_SZ) {
      tx_data[0] = 0;
      tx_cnt = 0;
      rx_cnt = 0;
    }
  } else {
    // receive data
    uint8_t data = USIDR;
    USIDR = 0;
    // We don't have to worry about whole-byte alignment because every
    // transaction starts with a large number of zeroes, which is guaranteed to
    // finish a previously unfinished transaction.
    if (!rx_rdy) {
      if (data != 0) {
        if (data == kvmd::MAGIC) {
          // start receive
          rx_rdy = true;
        } else if (!sync) {
          // We don't have enough pins for a CS, so we do a poor man's
          // synchronization by adjusting the phase until we detect our magic
          // word.
          USISR |= 0x0E << USICNT0;
        }
        sync = true;
      } else {
        sync = false;
      }
    }
    if (rx_rdy && rx_cnt < KVMD_MSG_SZ) {
      rx_data[rx_cnt] = data;
      ++rx_cnt;
    }
    if (rx_cnt == KVMD_MSG_SZ) {
      // stop receive
      rx_rdy = false;
    }
  }
}