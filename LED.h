#pragma once

#include "stdint.h"
#include "Utilities.h"
#include "Payload.h"
#include "Logging.h"

class LED
{
public:
	LED(Logger* logger, uint8_t pin);
	LED(uint8_t pin);
	~LED();

	//Logger _logger;

	uint8_t ledPin;

	uint8_t GetBrightness(Payload &p);
	void flashLED();
	void SetLED(uint8_t b);

};

