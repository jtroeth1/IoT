//to compile:
//g++ -o jora_gateway Jora.h Jora.cpp jora_gateway.cpp -lwiringPi -lbcm2835
#ifndef ARDUINO
#include <UNUSEDBUTFINE_TinyGPS++.h>
#include <cstdio>
#include "wiringPi.h"
#include "sql.h"
#include <time.h>
#include <chrono>
#include "FileRW.h"
#else
//#define printf Definitions::prf
#include "MemoryFree.h"
#include "TinyGPS++.h"
#include "GPS.h"
#include "SoftwareSerial.h"
#endif

#include "Jora.h"
#include "Utilities.h"
#include "Payload.h"
#include "LED.h"
#include "Definitions.h"
#include "Logging.h"
#include "Node.h"

bool receiveLock = false;
#ifndef ARDUINO
FileRW f;
Logger _logger(time(0));
#else
LED led(LED_PIN);
Logger _logger;
SoftwareSerial ss(GPS_RX, GPS_TX);
GPS gps(&ss);
#endif
Node node(&_logger);	//handles all processing things

//triggered on interrupt
void onReceive(uint8_t packetSize) {
	Serial.println("--Received payload--");

	//read packet
	for (uint8_t i = 0; i < packetSize; i++) {
		node.p.newMessage[i] = Jora.read();
		//node.p.payload[i] = Jora.read();
		_logger.log("%d", node.p.newMessage[i]);
	}
	_logger.log("\n");
	node.data.new_msg = true;
	receiveLock = false;
}

void setup() {
	_logger.log(Definitions::append, "Setting up --> ");
#ifdef ARDUINO
	Serial.begin(115200);
	node.data.dht.sensor.begin();
	ss.begin(GPS_BAUD);
	_logger.log(Definitions::execute, "Arduino ");
#ifdef MASTER
	_logger.log(Definitions::execute, "Master Node...");
#else
	pinMode(led.ledPin, OUTPUT);
	_logger.log(Definitions::execute, "Slave Node...");
#endif
#else
	_logger.log(Definitions::execute, "Master Gateway...");

	if (wiringPiSetup() < 0) {
		_logger.log("Failed to setup wiringPi...\n");
		while (true) {
			_logger.log(".");
			delay(1000);
		}
	}

#endif
	
	Jora.setPins(CS_PIN, RESET_PIN, IRQ_PIN);

	if (Jora.begin(DEFAULT_FREQ) < 0) {
		_logger.log("Jora begin failed!\n");	// ********************************* --> add while true
		while (true) {	//stay here
			_logger.log(".");
			delay(3000);
		}
	}

	

	//slaves will probably not use PA_BOOST
#ifndef ARDUINO
	Jora.setTxPower(TX_POWER, PA_OUTPUT_PA_BOOST_PIN);	//gateway uses inAir9B (PA_BOOST)
#else
	Jora.setTxPower(TX_POWER, PA_OUTPUT_RFO_PIN);
#endif

	//Set up onReceive callback ISR
	Jora.onReceive(onReceive);
	Jora.receive();	//set to receive mode

	_logger.log(Definitions::debug, "Jora CH: %#07x\n", Jora.getFrequency());

	_logger.log("Jora begin Success!\n");
	led.flashLED();	//setup success flash LED
	_logger.log("Free Memory: %d\n", freeMemory());

#ifndef ARDUINO
	//check that database and table already exist or create it
	if (!node.sql.executeCommand(node.sql.selectAll())) {
		node.sql.executeCommand(node.sql.createTable());
	}

	//Get ids
	int ret = node.GetIds();
	if (ret == SQL_QUERY_ERR) {
		_logger.log("Failed to check node ids!\n");
	}
	else if (ret == NODE_ID_NOT_SET_ERR) {
		_logger.log("Node id not set...\n");

		if (!node.SetNodeId()) {
			_logger.log("Failed to assign node ids!\n");
		}
		else {
			_logger.log("New Node id set!\n");
		}
}
	else {
		_logger.log("Node id already set: %d.%d.%d.%d\n", node.data.id[0], node.data.id[1], node.data.id[2], node.data.id[3]);
	}

	//check how many slave nodes already exist
	_logger.log("Checking slave nodes in db...\n");
	std::string q = "SELECT MAX(" + (std::string)NODE_ID_S + ") FROM " + (std::string)TABLE_NAME + " WHERE " + (std::string)CLASS_ID_S + " = " + std::to_string(SLAVE_CLASS_ID);
	char* cmd = new char[q.length() + 1];
	strcpy(cmd, q.c_str());
	node.sql.getNode(cmd);
	if (node.sql.buf != NULL)
		node.data.slave_node_total = std::stoi(node.sql.buf);
	else
		node.data.slave_node_total = 0;
	_logger.log(Definitions::debug, "There are %d slave nodes in database...\n", node.data.slave_node_total);
	node.sql.node_total = node.data.slave_node_total + 1;	//for now assume that there is only one master, later we can check


	node.lastSensorUpdateTime = time(0);	//set last update to current time
	node.lastSensorUpdateTime = node.lastSensorUpdateTime - Utilities::GetSecFromMin(NODE_SENSOR_UPDATE_TIME);	//subtract 10 mins to fake first update
	node.lastDownlinkUpdateTime = time(0);	//set last downlink to now and in at least 10 secs first check will occur

	node.p.ClearMessages();	//clear message buffer
	node.data.ClearCustomMessages();
	node.p.lastCMD = '#';	//no last cmd

#else
	node.p.ClearMessages();	//clear message buffer

	//Get ids
	if (!node.GetIds()) {
		Serial.println("Node id not set");

		if (!node.SetNodeId())
			Serial.println("Failed to assign node ids");

	}
	//set payload._id to ids
	node.p._id = &node.data.id[0]; 
	_logger.log("%d.%d.%d.%d\n", node.data.id[0], node.data.id[1], node.data.id[2], node.data.id[3]);
#endif

#ifdef ARDUINO
	_logger.log("Starting Jora Node...\n");

	gps.getCoords(node.data.lat_u.f, node.data.lon_u.f);	//get first lat lon
	Serial.print("lat: "); Serial.print(node.data.lat_u.f, 8);
	Serial.print(" lon: "); Serial.println(node.data.lon_u.f, 8);

	//do some checks
	if (node.data.id[2] != ID_NOT_SET && node.data.id[3] != ID_NOT_SET) {
		node.p.message[0] = Definitions::cmd::HANDSHAKE;
		node.p.SendMessage();
		node.data.awaiting = true;
	}

#else
	_logger.log(Definitions::debug, "Node id: %d.%d.%d.%d\n", node.data.id[0], node.data.id[1], node.data.id[2], node.data.id[3]);
	_logger.log("Starting Jora Gateway...\n");
#endif
}

#ifdef DEBUG
int lastFM = 1000;
int thisFM = -1;
int megaCounter = 0;
#endif

int fm = 0;
int count = 0;
void loop() {

	/*if (megaCounter % 100 == 0) {
		Serial.print("count: "); Serial.println(megaCounter);
	}	
	megaCounter++;
	*/
	/*int f = freeMemory();
	count++;
	if (fm != f || count > 10000) {
		_logger.log("Free Memory: %d\n", f);
		fm = f;
		count = 0;
	}*/
#ifndef ARDUINO
	if (Jora.newMessage)
		Jora.handleMessage();	//have to do this shit coz raspi interrupts are way quicker than arduino
#endif
	node.CheckNewMsgs();

#ifdef ARDUINO
#ifdef DEBUG
	thisFM = freeMemory();	//print memory remaining if it changes
	_logger.log("freeMemory: %d\n", thisFM);
	if (thisFM < lastFM) {
		Serial.print("freeMemory: "); Serial.println(thisFM);
		lastFM = thisFM;
	}
#endif
#endif

#ifdef MASTER
	//if awaiting reply or resend
	if (node.data.awaiting && node.data.resend_count != 4) {
		delay(2000);
		node.data.resend_count++;

	}
	else if (node.data.resend_count == 4) {
		node.data.resend = true;
	}

	//TESTING ************************
	//if not waiting for a reply, check if temp arrays or led arrays have been set
	//*********** need to fix the fact that if a node doesnt respond to a message from master, aster will just keep attempting to send message infinitely
	if (!node.data.awaiting) {

		//see if there is an outstanding 'ACK' to be fulfilled
		if (node.p.lastCMD != Definitions::BP) {
			switch (node.p.lastCMD) {
			case Definitions::ASSIGN_ID:
				node.sql.executeCommand(node.sql.insertNewNode(node.data.buf, -10.234, -33.234));
				node.data.clearBuf();
				break;
}
			node.p.lastCMD = Definitions::BP;
		}
		else {
			node.p.payloadSet = false;	//can overwrite the messages now ******************* --> TEST ME WITH ASSIGN_ID

			//check the current time against last update time
			time_t timeNow = time(0);
			time_t elapsedSUDT = timeNow - node.lastSensorUpdateTime; //time since last update		
			time_t elapsedDUDT = timeNow - node.lastDownlinkUpdateTime;
			//_logger.log("e: %d\n", elapsedDUDT);
			//if its been > 10mins then its time to get sensor update --> should use time from sql db "UD"
			if (elapsedSUDT > Utilities::GetSecFromMin(NODE_SENSOR_UPDATE_TIME) && node.data.activeSlaves > 0) {	//will assume that all update times are recent enough if this one is
																								//check temperatures
				if (node.data.slave_node_total > 0)	//if theres at least one node in the sql db
					node.CheckTemps();

				if (!node.data.getTemp) {	//this cant happen if the above happens first because the time gets reset in CheckTemps ****************** --> move somewhere else


					//node.CheckLeds(p, m);
				}

			}
			else if (elapsedDUDT > CHECK_DOWNLINK_TIME) {	//time is more than x seconds
				node.lastDownlinkUpdateTime = time(0);	//set last update to time now
				if (node.GetDownlinkMessage()) {	//check if theres any outstanding commands in text file
					_logger.log(Definitions::append, "Received downlink message: ");
					for (int i = 0; i < sizeof(node.data.dMessage) / sizeof(*node.data.dMessage); i++)
						_logger.log(Definitions::append, "%c", node.data.dMessage[i]);
					_logger.log(Definitions::execute, "");

					node.ConcatDownlinkMessage();	//extract string message into uint8_t payload

					//send message to node
					node.p.SendMessage();
				}
#ifdef DEBUG
				else {
					_logger.log(Definitions::debug, "No downlink messages available!\n");
				}
#endif
			}
		}
	}

#else
	if (node.data.awaiting && node.data.resend_count != 4) {
		if (!node.data.resend) {	//change the destination to master
			uint8_t* d = (uint8_t *)malloc(sizeof(uint8_t));
			d = Utilities::ConvertFromUint16_t(MASTER_NODE_ID);
			node.p.dest[0] = *(d);
			node.p.dest[1] = *(d + 1);
			free(d);
		}

		//handshake or id set hasnt occured yet
		if (node.data.id[2] == ID_NOT_SET && node.data.id[3] == ID_NOT_SET) {
			Serial.println("Waiting for id from master");
			node.p.message[0] = Definitions::cmd::ASSIGN_ID;
		}
		else if (!node.data.master_handshake) {
			Serial.println("Waiting for handshake from master");
			node.p.message[0] = Definitions::cmd::HANDSHAKE;
		}

		delay(2000);
		node.data.resend_count++;
	}
	else if (node.data.resend_count == 4) {
		node.data.resend = true;

	}
	else if(!node.data.awaiting) {
		if (node.data.lat_u.f == 0 || node.data.lon_u.f == 0) {	//first gps grab didnt work so we'll keep grabbing every loop until we have a valid gps
			//gps.getCoords(node.data.lat_u.f, node.data.lon_u.f);
			gps.getCoords();
		}
	}

#endif

	//Request the temp or set led if flag is set
#ifdef MASTER
	if (node.data.getTemp) {
		node.RequestTemp();
	}
	else if (node.data.setLED) {
		node.SetLed();
	}
#else
	if (!node.data.awaiting) {
		node.data.resend_count = 0;	//reset
		node.data.resend = false;

		if (node.data.getTemp) {
			Serial.println("Getting Temperature...");
			node.data.dht.GetAll(node.data.temp_u.f, node.data.hum_u.f);	
			node.data.dht.PrintAll(node.data.temp_u.f, node.data.hum_u.f);
			node.p.message[0] = Definitions::cmd::TEMP;

			node.data.FloatToBytes(node.data.temp_u.b, node.p.message, 1);	//add temperature float bytes into message
			node.data.FloatToBytes(node.data.hum_u.b, node.p.message, 1 + DATA_UNION_LEN);	//add humidity float bytes into message
			
			node.p.SendMessage();
			node.data.awaiting = true;
			
		}
		else if (node.data.setLED) {
			Serial.println("Setting LED...");
			led.SetLED(node.p.message[1]);
			node.p.message[0] = Definitions::cmd::LED;
			node.p.SendMessage();
			node.data.setLED = false;	//instead need this since led is now set
		}
		else if (node.data.getGPS) {
			_logger.log("Getting GPS Coords...\n");
			gps.getCoords(node.data.lat_u.f, node.data.lon_u.f);	//update coords
			node.data.FloatToBytes(node.data.lat_u.b, node.p.message, 1);	//add temperature float bytes into message
			node.data.FloatToBytes(node.data.lon_u.b, node.p.message, 1 + DATA_UNION_LEN);
			Serial.print("Lat: "); Serial.print(node.data.lat_u.f, 8);
			Serial.print(" Lon: ");Serial.println(node.data.lon_u.f, 8);

			node.p.message[0] = Definitions::cmd::GPS;
			node.p.SendMessage();

			node.data.awaiting = true;	//wait for ACK
		}
	}

#endif

	if (node.data.resend) {
		//resend
		node.p.SendMessage(true);
		node.data.resend = false;
		node.data.resend_count = 0;
	}

}

#ifndef ARDUINO
int main(int argc, char *argv[]) {

	setup();

	while (true) {
		loop();
	}


	printf("Finished..\n");
	return (0);
}
#endif