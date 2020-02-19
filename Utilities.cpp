#include "Utilities.h"

Utilities::Utilities()
{
}

bool Utilities::ByteToArray(uint8_t byte, uint8_t arr[], uint8_t index) {
	//Serial.print(" index: "); Serial.println(index);
	//Serial.print(" arr: ");
	for (int i = 2; i >= 0; i--) {
		arr[i + index] = byte % 10;
		 //Serial.print(arr[i + index]);
		byte /= 10;
	}

	return true;
}

uint8_t Utilities::ArrayToByte(uint8_t arr[], uint8_t index) {
	
	uint8_t byte = 0;
	index *= MESSAGE_CHAR_SIZE;
	for (uint8_t i = 0; i < MESSAGE_CHAR_SIZE; i++) {
		byte *= 10;
		byte += arr[i + index];
		
	}
	//_logger.log("%d, ", byte);
	return byte;
}

uint16_t Utilities::ConvertToUint16_t(uint8_t byte0, uint8_t byte1)
{
	//int b1 = (byte0 << 8) | byte1;
	//logger.log("b1: %d\n", b1);
	return (byte0 << 8) | byte1;
}

uint8_t* Utilities::ConvertFromUint16_t(uint16_t integer)
{
	uint8_t *ret = new uint8_t[2];
	
	*(ret) = ((integer >> 8) & 0xff);
	*(ret + 1) = (integer & 0xff);
	//logger.log("ret[0]: %d, ret[1]: %d\n", *(ret), *(ret + 1));
	return ret;

}

//returns a single char for printing eg node id 000.100.000.001 contains 4 byte arrays (12 bytes) therefor 12 chars individually returned
/*char Utilities::CharFromBytes(uint8_t byteArray[]) {
	//first get byte from array
	uint8_t b = ArrayToByte(byteArray, 0);
	char c = (char)(b + 48);
	printf("b: %d, c: %c", b, c);

	return c;
}*/

void Utilities::ConvertFromFloat(float in, uint8_t out)
{
	size_t size = sizeof(in);	//which will happen to be 4 bytes
	for (uint8_t i = 0; i < size; i++) {

	}
}

int Utilities::GetSecFromMin(int mins) {
	return mins * 60;
}

#ifndef ARDUINO
//Converts a string from "0.200.0.1" form to array of uint8_t
void Utilities::IdsFromString(std::string str, uint8_t dest[]) {
	std::vector<std::string> idStr;
	boost::split(idStr, str, [](char c) { return c == Definitions::cmd::DOT; });	//split at each '.'

	for (uint8_t i = 0; i < idStr.size(); i++) {
		//printf("%s == ", idStr[i].c_str());
		uint8_t n = 0;
		for (uint8_t j = 0; j < idStr[i].length(); j++) {
			n *= 10;
			n += (uint8_t)idStr[i][j] - 48;	//could be using std::stoi
			//printf("%d ", dest[i]);
		}
		dest[i] = n;
/*#ifdef DEBUG
		Logger::staticLog(1, "dest[%d]: %d\n", i, dest[i]);
#endif
*/
	}
}
#else
#include <stdarg.h>
#include <Arduino.h>

/*
void Utilities::prf(char *fmt, ...) {
	char buf[128]; // resulting string limited to 128 chars
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, (const char *)fmt, args); // progmem for AVR

	va_end(args);
	Serial.print(buf);
}
*/
//#define printf Utilities::prf
#endif

