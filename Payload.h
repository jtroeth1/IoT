#pragma once

#include "Definitions.h"
#include "Logging.h"
#include "Jora.h"
#include "Utilities.h"

#ifndef ARDUINO
#include <string>
#endif
#include <stdint.h>
#include "Definitions.h"

//cant include utilities

class Payload
{
private:
	Logger _logger;

public:
	Payload();
	~Payload();
	Payload(Logger* logger);

	uint8_t* _id;	//will always point to data.id[0]
	uint8_t dest[ID_SIZE];
	uint8_t sender[ID_SIZE];
	
	uint8_t length;
	uint8_t messageCount;
	char lastCMD;
	//uint8_t messageRaw[MAX_MESSAGE_LENGTH];	//message in raw bytes exc header
	uint8_t message[MAX_MESSAGE_LENGTH_CHAR];	//message with bytes joined
	uint8_t payload[PAYLOAD_SIZE];	//message including header
	uint8_t newMessage[PAYLOAD_SIZE];

	time_t lastSentTime;
	
	bool SendMessage(bool resend = false);
	
	//Packet ParseMessage(uint16_t node_id);
	bool ParseMessage();
	void PrintPayload();
	bool ConcatPayload(uint8_t val, uint8_t index);
	bool ClearMessages();
	void returnToSender();
	void SendACK();
	void SendReply(uint8_t cmd);
	void ParseFloat(uint8_t arrOut[], uint8_t offset, uint8_t len);

	enum payloadOrder {
		dest1,
		dest2,
		dest3,
		dest4,
		send1,
		send2,
		send3,
		send4,
		mCount,
		mLength,
		mPos
	};


private:
	bool checkRecipient(uint8_t id[]);
	bool CheckPayload();
	void printMessage();
	bool setPayload();
	void setMCount();
	bool ValidPayload();
	void ClearPayload();
};
