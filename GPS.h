#pragma once

#ifdef ARDUINO
#include "TinyGPS++.h"
#endif
#include "Definitions.h"
#include "Payload.h"

#ifdef ARDUINO
#include <SoftwareSerial.h>
#endif

class GPS
{
public:
	GPS(SoftwareSerial* _ss);
	GPS(float* lat, float* lon, SoftwareSerial* _ss);
	//void ParseGPS(Payload &p); //not using, use generic in payload:parsefloat

#ifdef ARDUINO
	TinyGPSPlus tgps;
	SoftwareSerial* ss;

	float lat, lon;

	void getCoords(float &lat, float &lon);
	void getCoords();
	void printFloat(float val);
	
	void ConcatGPS(uint8_t arr[]);	//takes from float into array of bytes
#endif

};
