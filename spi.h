#pragma once

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#define DO PB1  // MISO
#define DI PB0  // MOSI
#define SCK PB2 // SCLK

#define KVMD_MSG_SZ 8

void spi_usi_init();
bool spi_rx_get(uint8_t *data);
void spi_tx_write(const uint8_t *data);
