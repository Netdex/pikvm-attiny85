# pikvm-attiny85

Firmware for PiKVM "Arduino HID" SPI slave ported to ATtiny85

## Build
```bash
DIGISTUMP_PATH="C:/Users/$USER/AppData/Local/Arduino15/packages/digistump"
export ARDUINO_DIR="C:/Users/$USER/AppData/Local/Arduino15/packages/arduino"
export ARDUINO_LIB_PATH="$DIGISTUMP_PATH/hardware/avr/1.7.5/libraries"
export USER_LIB_PATH="$PWD/third-party"
export AVR_TOOLS_DIR="$ARDUINO_DIR/tools/avr-gcc/7.3.0-atmel3.6.1-arduino7"
export ALTERNATE_CORE_PATH="$DIGISTUMP_PATH/hardware/avr/1.7.5"
export MICRONUCLEUS_PATH="$DIGISTUMP_PATH/tools/micronucleus/2.6"

make upload
```

## Reference

- https://docs.pikvm.org/arduino_hid/
- https://github.com/pikvm/kvmd/tree/master/hid/arduino
