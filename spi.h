#pragma once

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#include "message.h"

#define DI PB0  // MOSI
#define DO PB1  // MISO
#define SCK PB2 // SCLK

#define KVMD_MSG_SZ 8

void spi_usi_init();
bool spi_rx_get(kvmd::message *msg);
void spi_tx_write(const kvmd::message *msg);
