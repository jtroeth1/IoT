#pragma once

#include "Logging.h"
#include "Definitions.h"
#include "Utilities.h"
#include "Payload.h"
#include "Data.h"

#ifndef ARDUINO
#include <wiringPi.h>
#include <cstdio>
#include <string.h>
#include <stdint.h>
#include <chrono>
#include <time.h>
#include "sql.h"
#include "FileRW.h"
#else
#include "EEPROM.h"
#include "Arduino.h"
//#define printf	Definitions::prf
#endif




class Node
{
private:
	Logger _logger;

public:
	Node();
	Node(Logger* logger);
	
#ifndef ARDUINO
	SqlClass sql;
	FileRW f;
#endif

	time_t lastSensorUpdateTime;	//time last update occured
	time_t lastDownlinkUpdateTime;
	Data data;
	Payload p;

	//both platforms but different
	bool GetIds();
	bool SetNodeId();

	//Gateway only
	bool DetermineNewNodeId(int arr[]);
	void CheckSqlData();
	void Request(char cmd);
	void ConcatDownlinkMessage();
	bool GetDownlinkMessage();
	void DisableNode();	//turn an active slave off
	void SendPing();

	//Arduino only
	void CheckIds(uint8_t &project_id, uint8_t &class_id, uint16_t &node_id);
	void prf(char* fmt, ...);	//printf

	//both but same
	void CheckNewMsgs();
	void SetLed();
	bool ProcessCmd();
	
};

