#include "Logging.h"
#include "Definitions.h"

/* Deprecated
#ifdef ARDUINO
#define printf Definitions::prf
#endif
*/

//normal event logger
Logger::Logger() {
	firstAppend = true;
	_header = "";
	_logType = Definitions::eventLogger;
}

Logger::Logger(std::string fileName, int logType, std::string header) : _fileName(fileName), _logType(logType), _header(header) {
	firstAppend = true;			
}

Logger::~Logger()
{
}

#ifndef ARDUINO
long Logger::GetFileSize(std::string filename) {
	struct stat stat_buf;
	int rc = stat(filename.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}
#endif

//used for logging and printing simultaneously (Default print style)
void Logger::EventLogger(char buf[]) {
#ifndef ARDUINO

	//get time
	time_t timeNow = time(0);
	tm *ltm = localtime(&timeNow);
	char timeBuf[80];
	strftime(timeBuf, 80, "%Y-%m-%d %H:%M:%S | ", ltm);	//format time into buf

	//convert char* to string
	std::string logStr(timeBuf);
	std::string bufStr(buf);

	logStr += bufStr;	//add str's

	if (logStr[logStr.length() - 1] != '\n')	//add new line if needed
		logStr += '\n';

	f.append(logStr, _fileName);	//log into log file

	//print to console
	printf("%s", logStr.c_str());
	

#else
	char buf[128]; // resulting string limited to 128 chars
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), (const char *)fmt, args); // progmem for AVR
	va_end(args);

	Serial.print(buf);

#endif
}

void Logger::log(int type, const char* fmt, ...) {
	//if (_logType == Definitions::logType::dataLogger)
	//	logAsDataLogger()

	char buf[80];	// !!!! can really fuck up if buf isnt big enough !!!!!!!!
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	//no type for data logger
	EventLogger(type, buf);
}

void Logger::log(int arr[], int len) {
	
	std::string line = "";
	for (int i = 0; i < len; i++) {
		line += std::to_string(arr[i]) + ',';		
	}
	char buf[line.length()];
	strcpy(buf, line.c_str());
	DataLogger(buf);
}

void Logger::log(const char* fmt, ...) {
	
	char buf[1024];	// !!!! can really fuck up if buf isnt big enough !!!!!!!!
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);	

	EventLogger(buf);							
}

void Logger::CheckHeader() {
	
	if (!f.CheckHeader(_fileName))
		f.append(_header + '\n', _fileName);

}

void Logger::DataLogger(char buf[]) {
	long fSize = GetFileSize(_fileName);
	if (fSize < 100)	//if a small file, check the size to see if it needs a header
		CheckHeader();
	else if (fSize > MAX_FILE_SIZE) {
		f.ArchiveLogFile(_fileName);
	}

	//get time
	time_t timeNow = time(0);
	tm *ltm = localtime(&timeNow);
	char timeBuf[80];
	strftime(timeBuf, 80, "%Y-%m-%d %H:%M:%S", ltm);	//format time into buf
	
	std::string logStr(timeBuf);
	std::string temp(buf);

	logStr += ',' + temp + '\n';

	f.append(logStr, _fileName);	
}

//overload used different logging styles
void Logger::EventLogger(int type, char buf[]) {
#ifndef ARDUINO
	//get time
	time_t timeNow = time(0);
	tm *ltm = localtime(&timeNow);
	char timeBuf[80];
	strftime(timeBuf, 80, "%Y-%m-%d %H:%M:%S | ", ltm);	//format time into buf

	//check log file size
	if (GetFileSize(_fileName) > MAX_FILE_SIZE) {
		//printf("Log file size: %ld\n", GetFileSize(LOG_FILENAME));
		f.ArchiveLogFile(_fileName);		
	}

	//convert char* to string - add time if first append
	std::string logStr;
	
	if (type == Definitions::append || type == Definitions::appendDebug){
		if (firstAppend) {
			std::string temp(timeBuf);
			logStr = temp;
			firstAppend = false;
			tempString = "";
		}
		else {
			logStr = "";
		}
	}
	else if (type == Definitions::execute || type == Definitions::executeDebug) {
		logStr = "";
	}
	else {
		std::string temp(timeBuf);
		logStr = temp;
	}
	
	std::string bufStr(buf);
	logStr += bufStr;	//add str's

	if ((type == 0 || type == 1) && logStr[logStr.length() - 1] != '\n')	//add new line if needed
		logStr += '\n';

	switch (type){
	case Definitions::log:	//normal log should also go into debug??
		//log only
		f.append(logStr, _fileName);
		printf("%s", logStr.c_str());
		break;

	case Definitions::debug:	//printf's in DEBUG mode only but still logs
		//debug only
		f.append(logStr, _fileName);	//log into log file
#ifdef DEBUG
		printf("%s", logStr.c_str());
#endif
		break;

	case Definitions::append:
		//append to tempString to log later		
		tempString += logStr;
		break;

	case Definitions::execute:
		//execute write of tempString to log file		
		tempString += logStr;
		if (tempString[tempString.length() - 1] != '\n')
			tempString += '\n';
		f.append(tempString, _fileName);
		printf("%s", tempString.c_str());
		tempString = "";
		firstAppend = true;
		break;

	case Definitions::appendDebug:
		tempStringDebug += logStr;
		break;

	case Definitions::executeDebug:
		tempStringDebug += logStr;
		if (tempStringDebug[tempStringDebug.length() - 1] != '\n')
			tempString += '\n';
		f.append(tempStringDebug, _fileName);	//log into log file
		tempStringDebug = "";
		firstAppend = true;
		break;
	}	

#else
	char buf[128]; // resulting string limited to 128 chars
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), (const char *)fmt, args); // progmem for AVR
	va_end(args);

#ifdef DEBUG
	if (type == Definitions::debug) {
		Serial.print(buf);
	}
#else
	//adjusted so append prints without new line, and execute with.
	//Not enough memory to store large strings or string is too large for serial.print
	//or char* being read from invalid address
	String logStr(buf);
	switch (type) {
	case Definitions::append:
		Serial.print(buf);
		break;

	case Definitions::execute:		
		Serial.println(buf);
		break;
	}
#endif
#endif
}
