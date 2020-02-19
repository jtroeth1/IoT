#include "Payload.h"

/*
#ifdef ARDUINO
#define printf	Definitions::prf
#endif
*/

int message_count = 0;

Payload::Payload() {
	lastSentTime = 0;
	for (uint8_t i = 0; i < sizeof(message) / sizeof(*message); i++) {
		message[i] = Definitions::cmd::BP;
	}
	//init vars to 0
	for (int i = 0; i < ID_SIZE; i++) {
		dest[i] = 0;
		sender[i] = 0;
	}
	length = 0;
	messageCount = 0;
}

Payload::Payload(Logger* logger) {
	lastSentTime = 0;
	_logger = *logger;
	for (uint8_t i = 0; i < sizeof(message) / sizeof(*message); i++) {
		message[i] = Definitions::cmd::BP;
	}
	//init vars to 0
	for (int i = 0; i < ID_SIZE; i++) {
		dest[i] = 0;
		sender[i] = 0;
	}
	length = 0;
	messageCount = 0;

	ClearMessages();	//clear message buffer
	lastCMD = '#';	//no last cmd
}

Payload::~Payload()
{
}

void Payload::PrintPayload() {
	_logger.log(Definitions::append, "New message from ");
	
	/*for (int i = 0; i < 4; i++) {
		_logger.log(Definitions::append, "%d", dest[i]);
		if (i != 3)
			_logger.log(Definitions::append, ".");
	}*/
	for (int j = 0; j < 4; j++) {
		_logger.log(Definitions::append, "%d", sender[j]);
		if (j != 3)
			_logger.log(Definitions::append, ".");
	}

	_logger.log(Definitions::append, " [%d, %d] ", messageCount, length);
	
	for (int i = 0; i < MAX_MESSAGE_LENGTH; i++) {
		if ((char)message[i] == Definitions::BP)
			break;
			
		_logger.log(Definitions::append, "%c", message[i]);			
	}
	_logger.log(Definitions::execute, "");
}

bool Payload::ParseMessage()
{
	if (!ValidPayload())
		return false;
	
	//Get the Header bytes from the payload	
	for (int i = 0; i < ID_SIZE; i++) {
		dest[i] = Utilities::ArrayToByte(payload, i);
		sender[i] = Utilities::ArrayToByte(payload, i + 4);
	}
	messageCount = Utilities::ArrayToByte(payload, payloadOrder::mCount);
	length = Utilities::ArrayToByte(payload, payloadOrder::mLength);
	
	//the rest is message	
	uint8_t temp = 0;
	for (int i = 0; i < MAX_MESSAGE_LENGTH; i++){	//rest of payload size
		int index = payloadOrder::mPos * MESSAGE_CHAR_SIZE + i;
		
		temp *= 10;
		temp += payload[payloadOrder::mPos * MESSAGE_CHAR_SIZE + i];// -CONVERT_CHAR_INT;

		if (i % 3 == 2) {			
			message[i / MESSAGE_CHAR_SIZE] = temp;						
			temp = 0;
		}
		
	}
		
	if (length != PAYLOAD_SIZE) {   // check length for error
		_logger.log(Definitions::debug, "Error: message length does not match expected length! (len=%d)\n", length);
		return false;
	}

	// if the recipient isn't this device or broadcast,
	if(!checkRecipient(_id)){
		_logger.log(Definitions::debug,  "This message is not for me! (Destination=%d.%d.%d.%d)\n", dest[0], dest[1], dest[2], dest[3]);
		return false;
	}

	PrintPayload();

	return true;

}

//make the destination the sender
void Payload::returnToSender() {
	uint8_t temp[4];
	for (int i = 0; i < 4; i++) {
		temp[i] = sender[i];
		sender[i] = dest[i];
		dest[i] = temp[i];
	}
}

bool Payload::checkRecipient(uint8_t id[]) {
	
	//_logger.log("my id is: %d.%d.%d.%d, the message is from %d.%d.%d.%d\n", id[0], id[1], id[2], id[3], dest[0], dest[1], dest[2], dest[3]);
	if (dest[2] == BROADCAST_ID && dest[3] == BROADCAST_ID) {	// = x.x.0.0
		if (dest[0] == id[0] || dest[1] == id[1])	//0.200.0.0
			return true;
	}
	else if (dest[0] == id[0] && dest[1] == id[1] && dest[2] == id[2] && dest[3] == id[3]) {
		return true;
	}

	return false;
}

bool Payload::ClearMessages() {
	//clear char message
	for (int i = 0; i < MAX_MESSAGE_LENGTH_CHAR; i++) {
		message[i] = Definitions::cmd::BP;
	}

	//Clear headers
	/*for (int i = 0; i < (PAYLOAD_SIZE - MAX_MESSAGE_LENGTH); i++) {
		payload[i] = Definitions::cmd::EOM;
	}*/

	return true;
}

void Payload::ClearPayload() {
	for (int i = 0; i < PAYLOAD_SIZE; i++) {
		payload[i] = 0;
	}
}

bool Payload::ConcatPayload(uint8_t val, uint8_t index) {
	//each payload section has three digits
	
	//p.payload[PAYLOAD_SIZE];//6 * 3 for header + 10 * 3 for message 
	index *= 3;
	Utilities::ByteToArray(val, payload, index);
	
	return true;
}

#ifdef ARDUINO
bool Payload::SendMessage(bool resend = false) {
	if (!resend) {
		/*
		uint8_t *d = Utilities::ConvertFromUint16_t(MASTER_NODE_ID);	//send message to master
		dest[2] = d[0];
		dest[3] = d[1];
		*/
		dest[1] = MASTER_CLASS_ID;	//send message to all masters use above to send rto specific addr
		setPayload();
	}
	else {
		_logger.log("Resending message...\n");
		ConcatPayload(message_count, payloadOrder::mCount);	//just update message count in message
	}
		
	
	Serial.print("Sending: ");
	for (int i = 0; i < PAYLOAD_SIZE; i++) {
		if (i < PAYLOAD_SIZE) {
			Serial.print((uint8_t)payload[i]);
		}
		else {
			Serial.print((char)payload[i]);
		}
		if (i % 3 == 2)
			Serial.print(" ");
	}
	Serial.println("");
	
	Jora.beginPacket();
	for (uint16_t i = 0; i < PAYLOAD_SIZE; i++) {
		Jora.write((char)payload[i]);
		delay(1);
	}
	Jora.endPacket();	
	
	message_count++;	//increment message count
	
	//Jora.flush();	//flush buffer (does nothing)
	
	Jora.receive();	//set back into receive mode

	return true;
}
#else

//get a float (byte array) to parse into union
void Payload::ParseFloat(uint8_t arrOut[], uint8_t offset, uint8_t len) {
	for (int i = 0; i < len; i++) {
		arrOut[i] = message[i - offset];
		_logger.log("arrOut: %d", arrOut[i]);
	}
}

void Payload::SendACK() {
	ClearMessages();	//rm message before overwriting first byte with ACK
	message[0] = Definitions::cmd::ACK;
	returnToSender();
	SendMessage();
	delay(500);	//make sure ACK has been received on the other end
}

void Payload::SendReply(uint8_t cmd) {
	ClearMessages();	//rm message before overwriting first byte with ACK
	message[0] = cmd;
	returnToSender();
	SendMessage();	
}

bool Payload::SendMessage(bool resend) {
	
	if (!resend) {
		setPayload();
		_logger.log("setting payload");
	}
	else {
		setMCount();	//just update message count
	}

	if (!ValidPayload()) {
		//ClearPayload();
		return false;
	}

	_logger.log(Definitions::append, "Sending | ");
	//printMessage();

	for (int i = 0; i < PAYLOAD_SIZE; i++) {
		if (i < PAYLOAD_SIZE) {
			_logger.log(Definitions::append, "%d", payload[i]);
		}
		else {
			_logger.log(Definitions::append, "%c", payload[i]);
		}
		if (i % 3 == 2)
			_logger.log(Definitions::append, " ");
	}
	_logger.log(Definitions::execute, "");

	Jora.beginPacket();
	for (int i = 0; i < PAYLOAD_SIZE; i++) {
		Jora.write(payload[i]);
		//delay(2);
	}
	Jora.endPacket();

	message_count++;	//increment message count

	Jora.receive();	//set back into receive mode

	lastSentTime = time(0);	//set time so that next message has to wait else its too quick for slave

	return true;
}

#endif

/*
void Payload::concatFloat(uint8_t arrIn[], uint8_t arrOut[]) {
	
	const uint8_t sz = sizeof(arrIn) / sizeof(*arrIn);

	for (int i = 0; i < sz; i++) {
		if (i < 4) {
			arrOut[i] = arrIn[i];
		}
		else {
			//Serial.print("d.humb: "); Serial.println(d.hum_u.b[i]);
			if (lon_u.f == 99.99) {
				arr[i] = 0;
			}
			else {
				arr[i] = lon_u.b[i - 4];
			}
		}
	}
}*/

bool Payload::setPayload() {

	for (int i = 0; i < ID_SIZE; i++) {
		ConcatPayload(dest[i], i);	//add destination
		ConcatPayload(_id[i], i + 4);	//add sender (id)
	}

	ConcatPayload(message_count, payloadOrder::mCount);
	ConcatPayload(PAYLOAD_SIZE, payloadOrder::mLength);

	//for(int i = 0; i < MAX_MESSAGE_LENGTH; i++)
	for (int i = 0; i < MAX_MESSAGE_LENGTH / MESSAGE_CHAR_SIZE; i++)	//rest of payload size
		ConcatPayload(message[i], payloadOrder::mPos + i);

	//printf("dest: %d.%d\n", dest[2], dest[3]);
	return true;
}

//update only message count
void Payload::setMCount() {
	ConcatPayload(message_count, payloadOrder::mCount);
}

//meant to be a byte but if the byte comes out more than 255 then it wasn't a byte
//could simply check if each byte is more than 9
bool Payload::ValidPayload() {
	_logger.log("payload validation disabled...");
	return true;

	for (int i = 0; i < PAYLOAD_SIZE; i++) {
		if (newMessage[i] > MAX_BYTE_SIZE) {
			_logger.log("Corrupt payload, discarding... ([%d]=%d)\n", i, newMessage[i]);			
			return false;
		}
		else {
			payload[i] = newMessage[i];
		}
	}

	return true;
}
//meant to be a byte but if the byte comes out more than 255 then it wasn't a byte
//could simply check if each byte is more than 9
/* -- deprecated -- (inefficient)
bool Payload::ValidPayload() {

	//first check the payload
	for (int i = 0; i < PAYLOAD_SIZE / MESSAGE_CHAR_SIZE; i++) {
		uint32_t byte = 0;	//need big int to check
		for (int j = 0; j < MESSAGE_CHAR_SIZE; j++) {
			byte *= 10;
			byte += newMessage[(i*3) + j];
			_logger.log("nm %d", newMessage[(i*3) + j]);
		}
		_logger.log("byte %d", byte);
		if (byte > SIZE_BYTE) {
			_logger.log("Corrupt payload, discarding...");
			ClearPayload();
			return false;
		}			
	}

	//copy new paylaod into payload array
	for (int i = 0; i < PAYLOAD_SIZE; i++) {
		payload[i] = newMessage[i];
	}

	return true;
}
*/


