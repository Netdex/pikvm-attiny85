#include "spi.h"

#include <avr/interrupt.h>
#include <util/atomic.h>
#include <wiring.h>

#include "protocol.h"

static volatile struct {
  volatile uint8_t rx_data[8] = {0};
  volatile uint8_t tx_data[8] = {0};

  volatile uint8_t rx_cnt = 0;
  volatile uint8_t tx_cnt = 0;
} state;

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

bool spi_rx_get(uint8_t *data) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (!state.tx_data[0] && state.rx_cnt == KVMD_MSG_SZ) {
      for (auto i = 0; i < KVMD_MSG_SZ; ++i) {
        data[i] = state.rx_data[i];
      }
      return true;
    }
    return false;
  }
}

void spi_tx_write(const uint8_t *data) {
  // no atomic needed because sentinel byte is written last
  for (int i = KVMD_MSG_SZ - 1; i >= 0; --i) {
    state.tx_data[i] = data[i];
  }
}

// USI interrupt routine. Always executed when 4-bit overflows (after 16 clock
// edges = 8 clock cycles):
ISR(USI_OVF_vect) {
  if (state.tx_data[0] && state.tx_cnt < KVMD_MSG_SZ) {
    // transmit data
    USIDR = state.tx_data[state.tx_cnt];
    ++state.tx_cnt;
    if (state.tx_cnt == KVMD_MSG_SZ) {
      state.tx_data[0] = 0;
      state.tx_cnt = 0;
      state.rx_cnt = 0;
    }
  } else {
    // receive data
    uint8_t data = USIDR;
    static bool rx_rdy = false;
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
    if (rx_rdy && state.rx_cnt < KVMD_MSG_SZ) {
      state.rx_data[state.rx_cnt] = data;
      ++state.rx_cnt;
    }
    if (state.rx_cnt == KVMD_MSG_SZ) {
      // stop receive
      rx_rdy = false;
    }
    USIDR = 0;
  }
  USISR = 1 << USIOIF;
}