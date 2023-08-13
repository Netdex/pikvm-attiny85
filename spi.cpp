#include "spi.h"

#include <avr/interrupt.h>
#include <wiring.h>

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

bool spi_rx_ready() { return !state.tx_data[0] && state.rx_cnt == KVMD_MSG_SZ; }
volatile uint8_t *spi_rx_get() { return state.rx_data; }

void spi_tx_write(const uint8_t *data) {
  for (int i = KVMD_MSG_SZ - 1; i >= 0; --i) {
    state.tx_data[i] = data[i];
  }
}

// USI interrupt routine. Always executed when 4-bit overflows (after 16 clock
// edges = 8 clock cycles):
ISR(USI_OVF_vect) {
  uint8_t in = USIDR;
  if (state.tx_data[0] && state.tx_cnt < KVMD_MSG_SZ) {
    USIDR = state.tx_data[state.tx_cnt];
    ++state.tx_cnt;
    if (state.tx_cnt == KVMD_MSG_SZ) {
      state.tx_data[0] = 0;
      state.tx_cnt = 0;
      state.rx_cnt = 0;
    }
  } else {
    static bool rx_rdy = false;
    if (!rx_rdy && in != 0) {
      rx_rdy = true;
    }
    if (rx_rdy && state.rx_cnt < KVMD_MSG_SZ) {
      state.rx_data[state.rx_cnt] = in;
      ++state.rx_cnt;
    }
    if (state.rx_cnt == KVMD_MSG_SZ) {
      rx_rdy = false;
    }
    USIDR = 0;
  }
  USISR = 1 << USIOIF;
}