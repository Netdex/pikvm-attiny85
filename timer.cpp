#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include <TrinketHidCombo.h>

// Marked ISR_NOBLOCK as required by USB driver
ISR(TIMER0_COMPA_vect) { TrinketHidCombo.poll(); }

void timer0_init() {
  // Set timer to CTC mode (Clear Timer on Compare Match)
  TCCR0A |= (1 << WGM01);

  // Set compare match value for desired timing
  OCR0A = 160; // Assuming 16.5MHz clock, 10ms with prescaler 1024

  // Set the prescaler to 1024
  TCCR0B |= (1 << CS02) | (1 << CS00);

  // Enable TIMER0 compare match A interrupt
  TIMSK |= (1 << OCIE0A);
}
