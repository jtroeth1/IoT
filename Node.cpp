#include "Node.h"

/*
Payload Node::p(Data _data) {
	return Payload(_data);
}
*/
/*
SqlClass Node::sql(Data _data)
{
	return SqlClass(_data);
}
*/

Node::Node(Logger* logger) {
	_logger = *logger;
	data = Data(logger);
	p = Payload(logger);

	lastSensorUpdateTime = time(0) - Utilities::GetSecFromMin(NODE_SENSOR_UPDATE_TIME);	//subtract 10 mins to fake first update
	lastDownlinkUpdateTime = time(0);	//set last downlink to now and in at least 10 secs first check will occur	
}

#ifndef ARDUINO

void Node::DisableNode() {
	//disable the node we've been resending to
	_logger.log("Disabling inactive node: %d.%d.%d.%d ...\n", p.dest[0], p.dest[1], p.dest[2], p.dest[3]);
	data.activeSlaves &= (uint16_t)(0 << Utilities::ConvertToUint16_t(p.dest[2], p.dest[3]));

	//Need a way to re-handshake with a node if it gets disabled
}

#ifdef MASTER
// this code loops forever to test speed with a node
time_t messageSentTime;
bool setup = false;
void Node::SendPing() {
	
	time_t timeNow = time(0);
	_logger.log("Sending Ping...");
	if (!setup) {
		p.message[0] = Definitions::cmd::PING;
		_logger.log("setting message[0] = P");
		p.dest[0] = 0;
		p.dest[1] = 200;
		p.dest[2] = 0;
		p.dest[3] = 1;

		p.SendMessage();
		messageSentTime = timeNow;

		setup = true;			
	}
	else {
		p.SendMessage(true);
	}			

	data.awaiting = true;
	
}
#else
void Node : PingTest() {

}
#endif
//Get id from database
bool Node::GetIds()
{
	//hardcoded set ids here from definitions
	data.id[0] = PROJECT_ID;
	data.id[1] = CLASS_ID;
	uint8_t* n = Utilities::ConvertFromUint16_t(MASTER_NODE_ID);
	data.id[2] = *n;
	data.id[3] = *(n + 1);

	//_logger.log("n[0]: %d, n[1]: %d --> = %d", *n, *n + 1, Utilities::ConvertToUint16_t(*n, *n + 1));

	//check if master node exists in db
	_logger.log("Checking Node id...\n");
	if (!sql.nodeExists(data.id))	//failed to check
		return false;
	//if (!sql.executeCommand(DATABASE_NAME, db, cmd))
	//return false;

	return true;
}

//Inserts a new node id into db
bool Node::SetNodeId() {
	//set ids in payload so that it doesn't need to be copied in every function call
	for (uint8_t i = 0; i < sizeof(data.id) / sizeof(*data.id); i++) {
		p._id[i] = data.id[i];
	}

	//insert master node into db
	_logger.log("Setting node id...\n");
	float lat = (float)-37.12836;
	float lon = (float)105.32434;
	if (!sql.executeCommand(sql.insertNewNode(data.id, lat, lon)))
		return false;

	return true;
}

//Create a new id for a node
bool Node::DetermineNewNodeId(int arr[]) {

	data.slave_node_total++;
	//_logger.log("new id: ");
	uint8_t* a = (uint8_t*)malloc(sizeof(uint8_t));
	a = Utilities::ConvertFromUint16_t(data.slave_node_total);
	arr[0] = *(a);
	arr[1] = *(a + 1);

	//_logger.log("arr[0]: %d, arr[1]: %d\n", arr[0], arr[1]);
	free(a);

	return true;
}

//check if need to request temperature from a node
void Node::CheckSqlData() {
	_logger.log("Checking SQL Data...\n");

	//check how many nodes with temps = null
	sql.sdTrig.class_id = 1;
	sql.sdTrigVals.class_id = SLAVE_CLASS_ID;
	if (!sql.getNode())	//data stored in sqlData struct
		return;

	if (sql.node_count > 0) {
		for (uint16_t nc = 1; nc < sql.node_count + 1; nc++) {
			//_logger.log("act: %d, nc: %d, &: %d\n", data.activeSlaves, nc, data.activeSlaves & nc);
			if ((data.activeSlaves & nc) == 1) {	//bit AND to see if slave in the db is currently active
				if (sql.sd[nc].lastUpdateTime > -1) {	//change me to a valid time needs to check if 0 or more than time + x
					data.getTemp = true;
					data.getGPS = true;
					sql.gpsCurrentNode = nc;
					sql.tempCurrentNode = nc;
					break;	//come back next time the elapsed time passes min. this will incrementally keep updating each node
				}
			}
		}
	}
}

//request some form of data from node
void Node::Request(char cmd) {
	uint8_t node = 0;
	switch (cmd) {
		case Definitions::TEMP:
			node = sql.tempCurrentNode - 1;	//index from 1 but use first array val
			break;

		case Definitions::GPS:
			node = sql.gpsCurrentNode - 1;
			break;
	}
	
	if (node < sql.node_count + 1) {	//indexing from 1 (0.0 is broadcast)
		_logger.log(Definitions::append, "Requesting %c", cmd);
		time_t elapsed = time(0) - sql.sd[node].lastUpdateTime;

		if (sql.sd[node].lastUpdateTime == 0 || elapsed > Utilities::GetSecFromMin(NODE_SENSOR_UPDATE_TIME)) {
			p.message[0] = cmd;

			p.dest[0] = sql.sd[node].project_id;
			p.dest[1] = sql.sd[node].class_id;
			uint8_t* n = Utilities::ConvertFromUint16_t(sql.sd[node].node_id);
			p.dest[2] = *n;
			p.dest[3] = *(n + 1);
			_logger.log(Definitions::execute, " from: %d.%d.%d.%d\n", p.dest[0], p.dest[1], p.dest[2], p.dest[3]);
			data.awaiting = true;

			p.SendMessage();
		}
	}
	else {
		switch (cmd) {
		case Definitions::TEMP:
			data.getTemp = false;
			break;

		case Definitions::GPS:
			data.getGPS = false;
		}
	}
}

//create a payload from downlink message
void Node::ConcatDownlinkMessage() {
	//will need updating for multiple commands in single downlink message
	std::vector<std::string> payload;

	boost::split(payload, data.downlinkMessage, [](char c) {return c == Definitions::cmd::CMD; });	//Split at each @

	for (uint8_t i = 0; i < payload.size(); i++) {

		if (payload[i][0] == Definitions::cmd::NI) {	//if '$'
			payload[i].erase(0, 1); //erase '$'
			Utilities::IdsFromString(payload[i], p.dest);	//extract array of ids from string and set as destination

		}
		else {
			//add the rest of message string to payload message
			for (uint8_t j = 0; j < payload[i].length(); j++) {
				p.message[j] = payload[i][j];
				//_logger.log("m: %d, %c\n", p.message[j], payload[i][j]);
			}
		}
	}
}

//Get downlink message from downlink.txt
bool Node::GetDownlinkMessage() {
	_logger.log(Definitions::debug, "Checking Downlinks...\n");
	std::string d;
	char dMess[MAX_MESSAGE_LENGTH];
	if (!f.GetDownlinkMessage(DOWNLINK_FILENAME, d, dMess)) {
		return false;
	}
	data.downlinkMessage = d;
	for (uint8_t i = 0; i < sizeof(dMess) / sizeof(*dMess); i++) {
		data.dMessage[i] = dMess[i];
		if (dMess[i] == Definitions::BP)
			break;
	}
	
	return true;
}

#else	//Arduino Only

bool Node::GetIds()
{
	//Read the ids from eeprom
	for (uint32_t addr = ID_ADDR; addr < ID_ADDR + 4; addr++) {
		uint8_t e = EEPROM.read(addr);
		*(data.id + (addr - ID_ADDR)) = e;
	}

	//Check if project id and class id have been set
	if (*(data.id) == ID_NOT_SET || *(data.id + 1) == ID_NOT_SET)
	{
		Serial.println("ids not set in EEPROM");
		Serial.println("writing project and class id...");
		EEPROM.write(ID_ADDR, PROJECT_ID);
		EEPROM.write((ID_ADDR + 1), CLASS_ID);
	}

	//Check if node id has been set yet or not
	if (*(data.id + 2) == ID_NOT_SET || *(data.id + 3) == ID_NOT_SET)
		return false;

	return true;
}

bool Node::SetNodeId() {
#ifdef MASTER
	//set to 0.200.0.1 --> master
	for (int i = 2; i < 4; i++) {
		EEPROM.write(ID_ADDR + i, (uint8_t)MASTER_NODE_ID);	//102 103
		*(ids + i) = MASTER_NODE_ID;
		//Serial.print(ID_ADDR + i); Serial.print((uint8_t)MASTER_NODE_ID);
	}
	_logger.log("Node id set\n");

	return true;
#else
	_logger.log("Node id: %d.%d.%d.%d\n", data.id[0], data.id[1], data.id[2], data.id[3]);

	if (data.id[2] == ID_NOT_SET || data.id[3] == ID_NOT_SET) {
		p.message[0] = Definitions::cmd::ASSIGN_ID;
		//Broadcast a no_id message to master
		p.SendMessage();

		Serial.println("No id msg sent to master");
		data.awaiting = true;
	}
	else {
		for (int i = 2; i < 4; i++) {
			EEPROM.write(ID_ADDR + i, *(data.id + i));	//102 103
		}
		Serial.print("Node id set.\n ");
		/*for (int i = 0; i < 4; i++) {
		Serial.print(EEPROM.read(ID_ADDR + i));
		if (i != 3)
		Serial.print(".");
		}*/

		//MASTER has designated id, therefore send an ACK
		if (p.messageCount > 0) {	//if mc more than 0 then must have sent master message to get id therfore ACK
			p.message[0] = Definitions::cmd::ACK;
			p.SendMessage();
		}

	}

	return true;
#endif
}

void Node::CheckIds(uint8_t &project_id, uint8_t &class_id, uint16_t &node_id) {
#ifdef MASTER
	if (class_id != MASTER_CLASS_ID || node_id != MASTER_NODE_ID) {
		Serial.println("Error: ids not valid for master");
		EEPROM.write(ID_ADDR + 2, 255);
		EEPROM.write(ID_ADDR + 3, 255);
		while (true);
	}
#else
	if (class_id != SLAVE_CLASS_ID || node_id == MASTER_NODE_ID) {
		Serial.println("Error: ids not valid for slave");
		EEPROM.write(ID_ADDR + 2, 255);
		EEPROM.write(ID_ADDR + 3, 255);
		while (true);
	}
#endif
}

#include <stdarg.h>
#include <Arduino.h>

void Node::prf(char *fmt, ...) {
	char buf[128]; // resulting string limited to 128 chars
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, (const char *)fmt, args); // progmem for AVR

	va_end(args);
	Serial.print(buf);
}
//#define printf Utilities::prf

#endif

void Node::CheckNewMsgs() {

	if (data.new_msg) {
		if (p.ParseMessage()) {
			ProcessCmd();
			
			//return true;
		}
		data.new_msg = false;
	}

	//return false;
}

void Node::SetLed() {
#ifdef MASTER
	_logger.log("Requesting node to set LED: %d.%d\n", p.dest[0], p.dest[1]);
	p.message[0] = Definitions::cmd::LED;
	p.message[1] = 200;	//brightness

	p.SendMessage();

	data.awaiting = true;
	data.setLED = false;

#else
	//enter LED changing code

#endif
}

//Process a sent command
bool Node::ProcessCmd() {
	//must grab commands before sending any replies else p.message will be written over.

	char cmd = p.message[0];	//have to add support later for multiple cmds
	/*if ((int)cmd < 32) {	//isnt a ascii char so print number
		_logger.log("cmd: %d\n", cmd);
	}
	else {
		_logger.log("cmd: %c\n", cmd);	//could be ascii char
	}*/
	switch (cmd) {
	case Definitions::cmd::ASSIGN_ID: {
		//a message is received that a node doesn't have an id
#ifdef MASTER
		if (p.sender[2] == ID_NOT_SET && p.sender[3] == ID_NOT_SET) {
			int arr[2];
			DetermineNewNodeId(arr);	//need to make it so save device gets same id back if message sends more than once
			p.message[0] = Definitions::ASSIGN_ID;

			for (int i = 0; i < 2; i++) {
				p.message[i + 1] = arr[i];
				//_logger.log("%d", arr[i]);

				data.buf[i + 2] = arr[i];	//store in buf to be used after ACK
				data.buf[0] = p.sender[0];
				data.buf[1] = p.sender[1];
			}

			p.returnToSender();
			p.SendMessage();	//send back to sender

			data.awaiting = true;
		}
		else {
			_logger.log("Node id from %d.%d.%d.%d already set!\n", p.sender[0], p.sender[1], p.sender[2], p.sender[3]);
		}

#else
		data.awaiting = false;
		if (data.id[2] == ID_NOT_SET || data.id[3] == ID_NOT_SET) {
			uint16_t temp = 0;
			for (int i = 1; i < 3; i++) {	//length of message for new node id (skip letter 'A')
				temp *= 10;
				temp += p.message[i];
			}

			//_logger.log("New Node id: %d\n", temp);
			uint8_t* t8 = (uint8_t*)malloc(sizeof(uint8_t));
			t8 = Utilities::ConvertFromUint16_t(temp);	//this has been changed yer
														//set ids to new node id
			*(data.id + 2) = *(t8); *(data.id + 3) = *(t8 + 1);
			//_logger.log("New id: %d.%d.%d.%d\n", *(id), *(id + 1), *(id + 2), *(id + 3));
			SetNodeId();	//write new node id into eeprom

			p.ClearMessages();	//clear messages

								//ACK to confirm new node set
			p.message[0] = Definitions::cmd::ACK;
			p.dest[2] = data.id[2]; p.dest[3] = data.id[3];	//change destination so next line returns to sender with new id
			p.returnToSender();
			p.SendMessage();
		}
		else {
			_logger.log("id already set!\n");
			//ACK to confirm new node set
			p.SendMessage();	//will send old message again
		}

#endif
	}
									  break;

	case Definitions::cmd::HANDSHAKE: {

#ifdef MASTER
		//check that the node is already in sql as it thinks it is
		sql.sdTrig.node_id = 1;
		sql.sdTrig.class_id = 1;	//trigger for class id and just 1 node from that class
		sql.sdTrigVals.node_id = Utilities::ConvertToUint16_t(p.sender[2], p.sender[3]);
		sql.sdTrigVals.class_id = SLAVE_CLASS_ID;
		if (!sql.getNode())
			return false;

	
		//add to db if not already
		if (!(sql.sd[0].node_id == Utilities::ConvertToUint16_t(p.sender[2], p.sender[3]))) {
			sql.executeCommand(sql.insertNewNode(p.sender, 0, 0));
			data.slave_node_total++;
		}
		
		//reply w handshake
		p.SendReply(Definitions::HANDSHAKE);
		
		//_logger.log("active: %d\n", data.activeSlaves);
		data.activeSlaves |= (1 << sql.sdTrigVals.node_id);	//need to add a func to check if slaves are still active. activeSlaves = 0;
	
		_logger.log(Definitions::debug, "Active Slaves: %d, Total: %d\n", data.activeSlaves, data.slave_node_total);

		delay(2000);	//delay after handshake

#else
		_logger.log("Handshake from master received!\n");
		data.master_handshake = true;
		data.awaiting = false;

#endif
	}
									  break;


	case Definitions::cmd::TEMP: {
		// need to check if the message contains data after cmd
#ifdef MASTER
		_logger.log("Received Temperature...\n");
		data.BytesToFloat(p.message, data.temp_u.b, 1);
		data.BytesToFloat(p.message, data.hum_u.b, 1 + DATA_UNION_LEN);
		
		_logger.log("Node: %d.%d.%d.%d ", p.sender[0], p.sender[1], p.sender[2], p.sender[3]);
		_logger.log("Temperature: %.2f ", data.temp_u.f);
		_logger.log("Humidity: %.2f\n", data.hum_u.f);			//// cant print floats!!

		//reply with ACK
		p.SendACK();
		
		//update sql
		if (data.temp_u.f > TEMP_INVALID || data.hum_u.f > TEMP_INVALID) {
			sql.executeCommand(sql.updateValue(p.sender, data.temp_u.f, TEMP_S));
		}
		data.awaiting = false;
		//adjust sqlNodeCount
		sql.tempCurrentNode++;
		data.getTemp = false;

#else

		data.getTemp = true;	//set bool to get temp in main

#endif
	}
								 break;
	case Definitions::cmd::ACK: {
#ifdef MASTER

		_logger.log("ACK confirmed!\n");
		data.awaiting = false;	//at this point data can be added to sql db
		p.ClearMessages();

#else
		Serial.println("Received ACK!");	//should only be sending one at a time
		if (data.getGPS)
			data.getGPS = false;
		if (data.getTemp)
			data.getTemp = false;
		if (data.setLED)
			data.setLED = false;
		
		data.awaiting = false;
#endif
	}
								break;

	case Definitions::cmd::LED: {
#ifdef MASTER
		//reply with ACK
		//p.message[0] = Definitions::cmd::ACK;
		//p.returnToSender();
		//p.SendMessage();

		_logger.log("Node %d.%d.%d.%d | LED set: %c", p.sender[0], p.sender[1], p.sender[2], p.sender[3], p.message[1]);

		//dont reply w ACK because slave just replied with ack if master receives this
		data.awaiting = false;
		data.setLED = false;
#else
		data.setLED = true;
#endif

	}
								break;

	case Definitions::cmd::SQL: {


	}
								break;

	case Definitions::cmd::GPS: {
#ifdef ARDUINO
		data.getGPS = true;
#else
		_logger.log("Received GPS...\n");
		data.BytesToFloat(p.message, data.lat_u.b, 1);
		data.BytesToFloat(p.message, data.lon_u.b, 1 + DATA_UNION_LEN);

		_logger.log("Node: %d.%d.%d.%d ", p.sender[0], p.sender[1], p.sender[2], p.sender[3]);
		_logger.log("Lat: %.8f ", data.lat_u.f);
		_logger.log("Lon: %.8f\n", data.lon_u.f);			//// cant print floats!!

		//reply with ACK		
		p.SendACK();
		
		//update sql
		if (data.lat_u.f != 0 && data.lon_u.f != 0) {
			_logger.log(Definitions::debug, "Updating db...\n");
			sql.executeCommand(sql.updateValue(p.sender, data.lat_u.f, GPS_LAT_S));
			sql.executeCommand(sql.updateValue(p.sender, data.lon_u.f, GPS_LON_S));
		}
		
		data.awaiting = false;

		//adjust sqlNodeCount
		sql.tempCurrentNode++;
		data.getGPS = false;
#endif
	}
						  break;

	case Definitions::cmd::PONG: {
		data.pongReceived = true;
		data.awaiting = false;
	}
								break;
	case Definitions::cmd::PING: {
		p.message[0] = Definitions::cmd::PONG;
		p.returnToSender();
	}
								 break;
	default: {
		_logger.log("Invalid cmd!\n");
	}
			 break;

	}

	if (cmd != Definitions::ACK)
		p.lastCMD = cmd;	//set this cmd to last so that we can check ACK

	return true;
}
