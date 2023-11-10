# ARDUINO_LIBS = DigisparkCDC
ARDUINO_LIBS = TrinketHidCombo

ALTERNATE_CORE = Digispark
BOARD_TAG = digispark-tiny
BOARD_SUB = default
VARIANT = digispark
F_CPU = 16500000L
ISP_PROG = micronucleus

# CPPFLAGS = -DKVMD_CDC_DEBUG

include third-party/arduino-mk/Arduino.mk