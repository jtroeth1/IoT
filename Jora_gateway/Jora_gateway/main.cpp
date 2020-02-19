//to compile:
//g++ -o jora_gateway Jora.h Jora.cpp jora_gateway.cpp -lwiringPi -lbcm2835
#ifndef ARDUINO
#include <cstdio>
#include "wiringPi.h"
#include "sql.h"
#include <time.h>
#include <chrono>
#include "FileRW.h"
#else
	#define printf Definitions::prf
#include "MemoryFree.h"
#include "SoftwareSerial.h"
#include "GPS.h"
#endif

#include "Jora.h"
#include "Utilities.h"
#include "Payload.h"
#include "LED.h"
#include "Definitions.h"
#include "Logging.h"
#include "Node.h"

Logger _logger;
//Node node;
Node node(&_logger);	//handles all processing things
#ifndef ARDUINO
FileRW f;
#else
//DHT	dht(DHTPIN, DHTTYPE);
LED led;
Logger _logger;
SoftwareSerial ss(GPS_RX, GPS_TX);
LED led;
#endif

//Test data rate between a node and gateway
#ifdef PINGTEST
bool pingTest = true;
Logger pingLogger("/home/master/IOT/Jora/log/pingTest.csv", Definitions::dataLogger , "DateTime,loopTime,RSSI");
#else
bool pingTest = false;
#endif

//triggered on interrupt
void onReceive(int packetSize) {
	_logger.log("--Received payload--\n");
	
	//read packet
	for (uint8_t i = 0; i < packetSize; i++) {
		node.p.newMessage[i] = Jora.read();	//store temporarily until message is validated, then set to paylaod
		_logger.log(Definitions::appendDebug, "%d", node.p.newMessage[i]);
		//node.p.payload[i] = Jora.read();
		//printf("%d", node.p.payload[i]);
	}
	_logger.log(Definitions::executeDebug, "\n");
	node.data.new_msg = true;
}

void setup() {
	_logger.log(Definitions::append, "Setting up --> ");
#ifdef ARDUINO
	Serial.begin(115200);
	dht.begin();
	ss.begin(GPS_BAUD);
	_logger.log(Definitions::execute, "Arduino...");
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
		_logger.log("Jora begin failed!\n");
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
	
#ifndef ARDUINO
	//check that database and node table already exist or create it
	if (!node.sql.executeCommand(node.sql.selectAll(NODE_TABLE))) {
		node.sql.executeCommand(node.sql.createNodeTable());
	}
	if (!node.sql.executeCommand(node.sql.selectAll(GW_TABLE))) {
		node.sql.executeCommand(node.sql.createGWTable());
	}

	//Get ids
	int ret = node.GetIds();
	if (ret == SQL_QUERY_ERR) {
		_logger.log("Failed to check node ids!\n");
	}
	else if(ret == NODE_ID_NOT_SET_ERR) {
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
	node.p._id = &node.data.id[0];	//set palyload id = node id (pointer so no need to update if change)

	//check how many slave nodes already exist
	_logger.log("Checking slave nodes in db...\n");
	node.data.slave_node_total = node.sql.setupNodes();	
	_logger.log("There are %d slave nodes in database...\n", node.data.slave_node_total);
	node.sql.node_total = node.data.slave_node_total + 1;	//for now assume that there is only one master, later we can check
		
	//node.lastSensorUpdateTime = time(0) - Utilities::GetSecFromMin(NODE_SENSOR_UPDATE_TIME);	//subtract 10 mins to fake first update
	//node.lastDownlinkUpdateTime = time(0);	//set last downlink to now and in at least 10 secs first check will occur

	//node.p.ClearMessages();	//clear message buffer
	//node.data.ClearCustomMessages();
	//node.p.lastCMD = '#';	//no last cmd
	
#else
	p.ClearMessages();	//clear message buffer

	//Get ids
	if (!node.GetIds(m.id)) {
		Serial.println("Node id not set");

		if (!node.SetNodeId(p, m.id, m))
			Serial.println("Failed to assign node ids");

	}
#endif

#ifdef ARDUINO
	_logger.log("Starting Jora Node...\n");
	gps.getCoords();	//get first lat lon
	printf("lat: %lf, lon: %lf\n", gps.lat, gps.lon);

	//do some checks
	//node.CheckIds(project_id, class_id, node_id);
	if (m.id[2] != ID_NOT_SET && m.id[3] != ID_NOT_SET) {
		//p.message[0] = Definitions::cmd::CMD;	//qualify with $	<-- whytf was i doing this??
		p.message[0] = Definitions::cmd::HANDSHAKE;
		p.SendMessage();
		m.awaiting = true;
	}
#else
	_logger.log(Definitions::debug, "Node id: %d.%d.%d.%d\n", node.data.id[0], node.data.id[1], node.data.id[2], node.data.id[3]);
	_logger.log("Starting Jora Gateway...\n");
	if (pingTest)
		_logger.log("-- Running Ping Test --\n");
#endif
}

void loop() {
	
#ifndef ARDUINO
	if (Jora.newMessage)	
		Jora.handleMessage();	//have to do this shit coz raspi interrupts are way quicker than arduino
#endif
	node.CheckNewMsgs();

#ifdef ARDUINO
#ifdef DEBUG
	Serial.print("freeMemory: "); Serial.println(freeMemory());
#endif
#endif

#ifdef MASTER
	//if awaiting reply or resend
	if (node.data.awaiting && node.data.resend_count != RESEND_LIMIT) {
		delay(2000);
		node.data.resend_count++;

	}
	else if (node.data.resend_count == RESEND_LIMIT) {
		node.data.resend = true;
	}

	//Update Temps, GPS, Check downlinks
	if (!node.data.awaiting) {
		node.data.resend_retries = 0;	//reset
		node.data.resend_count = 0;

		bool sendElapsed = false;
		time_t timeNow = time(0);
		time_t elapsedSinceLastMessage = timeNow - node.p.lastSentTime; //time since last message was sent
		if (elapsedSinceLastMessage > MIN_SEND_TIME || node.p.lastSentTime == 0 || pingTest)	//limit sending to once per second
			sendElapsed = true;		

		//see if there is an outstanding 'ACK' to be fulfilled
		if (node.p.lastCMD != Definitions::BP) {
			switch (node.p.lastCMD) {
			case Definitions::ASSIGN_ID:
				node.sql.executeCommand(node.sql.insertNewNode(node.data.buf, -10, -33));
				node.data.clearBuf();
				break;
			}
			node.p.lastCMD = Definitions::BP;
		}
		else if (sendElapsed) {//Request the temp or gps or set LED
			if (pingTest && node.data.activeSlaves) {
				if (node.data.pongReceived)
					node.SendPing();
			}
			else
			{
				if (node.data.getTemp) {
					node.Request(Definitions::TEMP);
				}
				else if (node.data.getGPS) {
					node.Request(Definitions::GPS);
				}
				else if (node.data.setLED) {
					node.SetLed();
				}
				else {
					//check the current time against last update time
					time_t timeNow = time(0);
					time_t elapsedSUDT = timeNow - node.lastSensorUpdateTime; //time since last update		
					time_t elapsedDUDT = timeNow - node.lastDownlinkUpdateTime;

					//if its been > 10mins then its time to get sensor update --> should use time from sql db "UD"
					if (elapsedSUDT > Utilities::GetSecFromMin(NODE_SENSOR_UPDATE_TIME) && node.data.activeSlaves > 0) {	//will assume that all update times are recent enough if this one is
						//_logger.log("elapsed: %d %d", elapsedSUDT, Utilities::GetSecFromMin(NODE_SENSOR_UPDATE_TIME));			//check temperatures

						if (node.data.activeSlaves > 0) {	//if theres at least one active slave
							node.CheckSqlData();	//Check who/what needs updating in SQL
							node.lastSensorUpdateTime = time(0);	//update last update time so that next check is in 10 mins (even if didnt gather any temps)
						}
						/*else if(node.data.slave_node_total > 0){
							//ping the device with a handshake that are in db but not
						}*/
					}
					else if (elapsedDUDT > CHECK_DOWNLINK_TIME) {	//time is more than x seconds
						node.lastDownlinkUpdateTime = time(0);	//set last update to time now

						if (node.data.activeSlaves > 0) {
							if (node.GetDownlinkMessage()) {	//check if theres any outstanding commands in text file
								_logger.log(Definitions::append, "Received downlink message: ");
								for (uint8_t i = 0; i < sizeof(node.data.dMessage) / sizeof(*node.data.dMessage); i++)
									_logger.log(Definitions::append, "%c", node.data.dMessage[i]);
								_logger.log(Definitions::execute, "");

								node.ConcatDownlinkMessage();	//extract string message into uint8_t payload

								//send message to node
								node.p.SendMessage();
							}
						}
					}

#ifdef DEBUG
					else {
						_logger.log(Definitions::debug, "No downlink messages available!\n");
					}
#endif
				}
			}
		}

	}


#else
	if (m.awaiting && m.resend_count != 4) {
		if (!node.data.resend) {
			uint8_t* d = (uint8_t *)malloc(sizeof(uint8_t));
			d = Utilities::ConvertFromUint16_t(MASTER_NODE_ID);
			p.dest[0] = *(d);
			p.dest[1] = *(d + 1);
			free(d);
		}

		if (m.id[2] == ID_NOT_SET && m.id[3] == ID_NOT_SET) {
			Serial.println("Waiting for id from master");
			p.message[0] = Definitions::cmd::ASSIGN_ID;
		}
		else if (!m.master_handshake) {
			Serial.println("Waiting for handshake from master");
			p.message[0] = Definitions::cmd::HANDSHAKE;
		}

		delay(2000);
		m.resend_count++;
	}
	else if (m.resend_count == 4) {
		m.resend = true;

	}
	else if(!m.awaiting) {
		if (gps.lat == 0 || gps.lon == 0) {	//first gps grab didnt work so we'll keep grabbing every loop until we have a valid gps
			//printf("Getting coords\n");
			gps.getCoords();
		}
	}

	if (!m.awaiting) {
		if (m.getTemp) {
			Serial.println("Getting temp...");
			m.tempSensor.GetAll(dht);	//returns true always

			m.tempSensor.PrintAll(dht);
			p.message[0] = Definitions::cmd::TEMP;
			uint8_t temp_arr[sizeof(DHTData::temp_u.b) * 2];	//also humidity
			m.tempSensor.ConcatTemp(temp_arr);
			printf("sizeof temp_u.b: %d\n", sizeof(DHTData::temp_u.b) * 2);
			for (int i = 1; i < sizeof(temp_arr) + 1; i++) {
				p.message[i] = temp_arr[i];
				//printf("%d ", temp_arr[i]);
				//p.payloadSet = false;
			}
			p.SendMessage();
			m.awaiting = true;
			
		}
		else if (m.setLED) {
			node.SetLed(p, m);
				Serial.println("Setting LED...");
			//led.SetLED(led.GetBrightness(p));
			led.SetLED(p.message[1]);
			p.message[0] = Definitions::cmd::LED;
			p.SendMessage();
			m.awaiting = true;
		}
	}

#endif

	if (node.data.resend) {
		//resend		
		node.p.SendMessage(true);
		node.data.resend = false;
		node.data.resend_count = 0;
		node.data.resend_retries++;

		if (node.data.resend_retries > MAX_RESEND_RETRIES) {
			node.DisableNode();	//disables the destination node
			node.data.resend_retries = 0;
			node.data.awaiting = false;
		}
	}
	
}


void pingLoop() {
	auto timeStartLoop = std::chrono::high_resolution_clock::now();
	//send message
	std::string m = "p";
	Jora.beginPacket();
	for (int i = 0; i < m.length(); i++)
		Jora.write(m[i]);
	Jora.endPacket();
	std::cout << "ping sent, waiting for reply..." << std::endl;
	auto tStartSend = std::chrono::high_resolution_clock::now();
	

	//wait for reply
	Jora.receive();
	bool messageReceived = false;
	time_t timeStartWait = time(0);
	
	while (!messageReceived) {
		//printf("nm: %d\n", Jora.newMessage);
		if (Jora.newMessage) {
			Jora.handleMessage();	//have to do this shit coz raspi interrupts are way quicker than arduino				
			messageReceived = true;

			int messageIn[m.length()];
			for (int i = 0; i < m.length(); i++) {
				messageIn[i] = node.p.newMessage[i];
				printf("%c", messageIn[i]);
			}
			printf("\n");

			//auto tEndSend = std::chrono::high_resolution_clock::now();			
			//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(tStartSend - tEndSend).count();
			//std::cout << "Full duplex time: " << duration << std::endl;
		}

		time_t elapsedWait = time(0) - timeStartWait; //break if message not received after length of time	
		if (elapsedWait >= 2) {
			printf("elapsed: %d\n", elapsedWait);
			break;
		}
			
	}	

	
	auto timeEndLoop = std::chrono::high_resolution_clock::now();
	auto loopDuration = std::chrono::duration_cast<std::chrono::microseconds>(timeStartLoop - timeEndLoop).count();
	std::cout << "Loop time: " << loopDuration / 1000 << "ms" << std::endl;	
	
	int arr[2];
	arr[0] = loopDuration;
	arr[1] = Jora.packetRssi();
	pingLogger.log(arr, 2);
	

	//delay(50);
}

#ifndef ARDUINO
int main(int argc, char *argv[]) {

	setup();

	if (pingTest) {
		while (true) {
			try {
				pingLoop();
				//delay(2);
			}
			catch (const std::exception& e) {
				std::cout << e.what();
			}
			
		}
	}

	while (true) {
		try {
			loop();
			delay(2);	//slow it down baby
		}
		catch (const std::exception& e){
			std::cout << e.what();
		}
		
	}
	
	return (0);
}
#endif

//  a == &a[0]
//  *a == a[0]
//  a+i == &a[i]
//  *(a+i) == a[i]

//print time
/*struct tm *tmpNow = localtime(&timeNow);
struct tm *tmpLastUpdate = localtime(&node.lastUpdateTime);
char buf[80];
strftime(buf, sizeof(buf), "%H:%M:%S", tmpNow);
//puts(buf);
*/