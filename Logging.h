#pragma once

#include "stdarg.h"
#include "time.h"
#include "Definitions.h"

#ifndef ARDUINO
#include "FileRW.h"
#else
#include "Arduino.h"
#endif

class Logger
{
public:
	Logger();
	Logger(std::string fileName, int logType, std::string header);	//provide the full file path	
	~Logger();

#ifndef ARDUINO
	FileRW f;	//for logging to file
	//bool checkLogAge();	
	std::string tempString;	//stores the log string until type 4 (execute log append)
	std::string tempStringDebug; //stores the log string for debug
	//void logToFile(std::string);	//logs the string to a log file
#else
	String tempString;
#endif
	
	void log(const char* fmt, ...);	//replaces printf calls
	void log(int type, const char* fmt, ...);	//use type to modify logging style
	void log(int arr[], int len);
	

private:
	bool firstAppend;
	std::string _header;	//only used for datalogger
	std::string _fileName;
	uint8_t _logType;
	void EventLogger(char buf[]);	//replaces printf calls
	void EventLogger(int type, char buf[]);	//use type to modify logging style
	
	void DataLogger(char buf[]);
	void CheckHeader();	//returns true if header exists

#ifndef ARDUINO
	long GetFileSize(std::string filename);
#endif
	//void prf(char* fmt, ...);
};

