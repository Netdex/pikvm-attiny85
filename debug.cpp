#include "debug.h"

#include <stdarg.h>
#include <stdio.h>

#ifdef KVMD_CDC_DEBUG
#include <DigiCDC.h>

void _debug(const char *fmt, ...) {
  static char buf[32] = {0};
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  SerialUSB.write(buf);
}
#endif