#include "Data.h"

Data::Data()
{
	setLED = false;
	getTemp = false;
	getGPS = false;
	awaiting = false;
	new_msg = false;
	pongReceived = true;

#ifndef ARDUINO
	activeSlaves = 0;
	slave_node_total = 0;
#endif
}

Data::Data(Logger* logger) {
	
	_logger = *logger;
	setLED = false;
	getTemp = false;
	getGPS = false;
	awaiting = false;
	new_msg = false;
	pongReceived = true;
	dht = DHTData(logger);
	
	

#ifndef ARDUINO
	ClearCustomMessages();
	activeSlaves = 0;
	slave_node_total = 0;
#endif
}


Data::~Data() {
}

void Data::ConcatZeros(uint8_t arrIn[], uint8_t arrOut[], uint8_t offset, uint8_t len) {
	for (int i = 0; i < len + offset; i++)
		arrOut[i] = 0;
}

//For use with DataUnion
void Data::FloatToBytes(uint8_t dataIn[], uint8_t arrOut[], uint8_t offset) {
	for (int i = 0; i < DATA_UNION_LEN; i++) {
		arrOut[i + offset] = dataIn[i];
	}
}
//For use with DataUnion
void Data::BytesToFloat(uint8_t dataIn[], uint8_t arrOut[], uint8_t offset) {
	for (int i = 0; i < DATA_UNION_LEN; i++) {
		arrOut[i] = dataIn[i + offset];
	}
}

#ifndef ARDUINO
void Data::ClearCustomMessages() {
	for (uint8_t i = 0; i < sizeof(dMessage) / sizeof(*dMessage); i++)
		dMessage[i] = Definitions::BP;
}
#endif

void Data::clearBuf() {
	uint8_t size = sizeof(buf) / sizeof(buf[0]);
	for (int i = 0; i < size; i++) {
		buf[i] = '#';
	}
}
