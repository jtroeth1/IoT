#pragma once

#include "Logging.h"
#include "Payload.h"

#ifdef ARDUINO
#include "DHT.h"
#endif

class DHTData
{
private:
	Logger _logger;

public:
	DHTData();
	DHTData(Logger* logger);
	
	~DHTData();

#ifdef ARDUINO
	DHT sensor = DHT(DHTPIN, DHTTYPE);
#endif

#ifdef ARDUINO
	bool GetAll(float &temp, float& hum);
	void PrintAll(float &temp, float &hum);
#endif
	bool ParseTemp(Payload &p);
	bool ConcatTemp(uint8_t arr[]);

};