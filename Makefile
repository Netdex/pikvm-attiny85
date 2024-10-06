BOARD_TAG = digispark-tiny
BOARD_SUB = default
VARIANT = digispark
F_CPU = 16500000L
ISP_PROG = micronucleus

ARDUINO_LIBS = TrinketHidCombo

# We don't use any part of the Arduino core, so we can technically build with
# AVR libraries alone.
# NO_CORE = True
# MCU = attiny85
# HEX_MAXIMUM_SIZE = 6650

include third-party/arduino-mk/Arduino.mk

compdb:
	$(MAKE) clean && rm -f compile_commands.json
	compiledb $(MAKE)