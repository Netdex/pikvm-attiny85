#pragma once

#ifdef KVMD_CDC_DEBUG
#define DEBUG(msg, ...) _debug(msg, __VA_ARGS__)
void _debug(const char *fmt, ...);
#else
#define DEBUG(msg, ...) (void)msg
#endif