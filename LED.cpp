
#include "LED.h"

LED::LED(uint8_t pin) : ledPin(pin){
	
}

LED::LED(Logger* logger, uint8_t pin) : ledPin(pin) {
	//_logger = *logger;
}

LED::~LED()
{
}


uint8_t LED::GetBrightness(Payload &p) {
	uint8_t arr[3];
	for (uint8_t i = 1; i < sizeof(arr) + 1; i++) {
		arr[i - 1] = p.message[i];
	}
	
	return Utilities::ArrayToByte(arr, 0);
}

void LED::flashLED() {
	for (int i = 0; i < 2; i++) {
		analogWrite(ledPin, 255);
		delay(100);
		analogWrite(ledPin, 0);
		delay(75);
	}
}

void LED::SetLED(uint8_t b) {
	//_logger.log("Setting LED Brightness: %d\n", b);
	uint8_t brightness = (b - (uint8_t)48);
	//_logger.log("Setting LED Brightness: %d / %d\n", b, brightness);
	if (brightness == 1)
		brightness = 255;
	else
		brightness = brightness * (uint8_t)255;	//can control brightness but easier just to send 1 for on
	//write value
	//_logger.log("pin: %d, Setting LED Brightness: %d / %d\n", ledPin, b, brightness);
#ifdef ARDUINO
	analogWrite(ledPin, brightness);
#endif
}
	