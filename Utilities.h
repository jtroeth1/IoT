#pragma once

#include "Definitions.h"
#include "Logging.h"

#ifndef ARDUINO
#include <boost/algorithm/string.hpp>
#include <vector>
#else
//#define printf Definitions::prf
#include <SoftwareSerial.h>
#endif

class Utilities
{
private:
	//Logger _logger;
	static Logger logger;	//static so it can be used in static class

public:
	Utilities();
	
	//both platforms but same
	static uint16_t ConvertToUint16_t(uint8_t byte1, uint8_t byte2);
	static uint8_t* ConvertFromUint16_t(uint16_t integer);
	static bool ByteToArray(uint8_t byte, uint8_t arr[], uint8_t index);
	static uint8_t ArrayToByte(uint8_t arr[], uint8_t index);
	static int GetSecFromMin(int mins);
	static void ConvertFromFloat(float in, uint8_t out);
	//Gateway only
#ifndef ARDUINO
	static void IdsFromString(std::string, uint8_t arr[]);
	static char CharFromBytes(uint8_t arr[]);
#else
	//Arduino only
	void prf(char *fmt, ...);
#endif

};