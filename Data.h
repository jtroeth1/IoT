#pragma once

#ifndef ARDUINO
#include <string>
#endif
#include "Definitions.h"
#include <stdint.h>
#include "DHTData.h"
#include "Logging.h"

class Data
{
private:
	Logger _logger;
	

public:
	Data();
	Data(Logger* logger);
	~Data();
	uint8_t id[ID_SIZE];	//node id

	union DataUnion
	{
		float f;	//point to dht.temperature or gps.lat etc
		uint8_t b[DATA_UNION_LEN];
	};
	DataUnion temp_u, hum_u, lat_u, lon_u;
	
	//move to sql db
#ifndef ARDUINO
	uint16_t activeSlaves;	//current active (bitwise) --> this will cause issues if master dies but and restarts and nodes think they have hand shakedded
	uint8_t slave_node_total; // total active, no more than 255 at this stage
#endif
	DHTData dht;
	uint8_t buf[10];	//store random data for use after ACK eg. determine new node id, id gets stored here.

	bool setLED, getTemp, getGPS, awaiting, new_msg, pongReceived;

	void FloatToBytes(uint8_t dataIn[], uint8_t arrOut[], uint8_t offset);
	void BytesToFloat(uint8_t dataIn[], uint8_t arrOut[], uint8_t offset);
	void ConcatZeros(uint8_t arrIn[], uint8_t arrOut[], uint8_t offset, uint8_t len);

#ifdef ARDUINO
	bool master_handshake = false;
#endif

	uint8_t resend_count = 0;
	bool resend = false;

#ifndef ARDUINO
	char dMessage[MAX_MESSAGE_LENGTH];	//custom command message buffer
	std::string downlinkMessage;
	void ClearCustomMessages();
	uint8_t resend_retries = 0;
#endif

	void clearBuf();

};
