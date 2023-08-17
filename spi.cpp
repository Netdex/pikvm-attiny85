#include "spi.h"

#include <avr/interrupt.h>
#include <util/atomic.h>
#include <wiring.h>

#include "message.h"
#include "protocol.h"

namespace {
kvmd::message_buffer rx_buffer;

bool rx_rdy = false;
bool tx_rdy = false;
volatile uint8_t tx_data[8] = {0};
uint8_t rx_data[8] = {0};
uint8_t rx_cnt = 0;
uint8_t tx_cnt = 0;
} // namespace

void spi_usi_init() {
  cli();               // Deactivate interrupts
  DDRB &= ~(1 << DI);  // Set DI as input
  DDRB &= ~(1 << SCK); // Set SCK as input
  DDRB |= 1 << DO;     // MISO Pin has to be an output

  USICR = ((1 << USIWM0) | (1 << USICS1) |
           (1 << USIOIE)); // Activate 3- Wire Mode and use of external clock
                           // and enable overflow interrupt

  delay(500); // Let things settle
  sei();      // Activate interrupts
}

bool spi_rx_get(kvmd::message *msg) { return rx_buffer.next(msg); }

void spi_tx_sticky_write(kvmd::message *msg) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    for (auto i = 0; i < KVMD_MSG_SZ; ++i) {
      tx_data[i] = msg->bytes[i];
    }
  }
}

// USI interrupt routine. Always executed when 4-bit overflows (after 16 clock
// edges = 8 clock cycles):
ISR(USI_OVF_vect) {
  if (tx_rdy && tx_cnt < KVMD_MSG_SZ) {
    // transmit data
    USIDR = tx_data[tx_cnt];
    ++tx_cnt;
    if (tx_cnt == KVMD_MSG_SZ) {
      tx_cnt = 0;
      rx_cnt = 0;
      tx_rdy = false;
    }
  } else {
    // receive data
    uint8_t data = USIDR;
    if (!rx_rdy && data != 0) {
      if (data != kvmd::MAGIC) {
        // clear flags, shift 1 clock cycle
        USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) |
                (0x0E << USICNT0);
      } else {
        // start receive
        rx_rdy = true;
      }
    }
    if (rx_rdy && rx_cnt < KVMD_MSG_SZ) {
      rx_data[rx_cnt] = data;
      ++rx_cnt;
    }
    if (rx_cnt == KVMD_MSG_SZ) {
      // stop receive
      rx_rdy = false;
      tx_rdy = true;
      rx_buffer.push(reinterpret_cast<kvmd::message *>(rx_data));
    }
    USIDR = 0;
  }
  USISR = 1 << USIOIF;
}