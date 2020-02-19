#include "Definitions.h"

Definitions::Definitions()
{
}

#ifdef ARDUINO
#include <stdarg.h>
#include <Arduino.h>
//printf f
/*
void Definitions::prf(char *fmt, ...) {
	char buf[128]; // resulting string limited to 128 chars
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf),(const char *)fmt, args); // progmem for AVR

	va_end(args);
	Serial.print(buf);
}
*/
#endif
