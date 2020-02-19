#include "DHTData.h"

#ifdef ARDUINO
#include "DHT.h"
#include "Definitions.h"
//#define printf	Definitions::prf
#else
#include <cstdio>
#include <cmath>
#include <stdint.h>
#endif

#ifdef ARDUINO
#define nanCheck	isnan
#else
#define nanCheck	std::isnan
#endif

#ifdef ARDUINO
DHTData::DHTData() {
}

DHTData::DHTData(Logger* logger) {
	_logger = *logger;
	sensor = DHT(DHTPIN, DHTTYPE);
}
#else
DHTData::DHTData()
{
	
}
DHTData::DHTData(Logger* logger) {
	_logger = *logger;
}
#endif


DHTData::~DHTData()
{
}

#ifdef ARDUINO
bool DHTData::GetAll(float &temperature, float &humidity) {

	float temp = 0; //sensor.readTemperature();
	float hum = 0;// sensor.readHumidity();

	Serial.println("Temp Sensor Broken!");

	if (isnan(temp) || isnan(hum)) {
		Serial.println("Failed to read from DHT sensor!");
		return true;	//return true here and just send nan
	}

	temperature = temp;
	humidity = hum;

	//Serial.println(temp);

	return true;
}

void DHTData::PrintAll(float &temp, float &hum) {
	Serial.print("Temp: "); Serial.print(temp); Serial.print(" C");
	Serial.print(" Humidity: "); Serial.println(hum);
}
#endif

bool DHTData::ConcatTemp(uint8_t arr[]) {
	
	//temp_u.f = temp;
	//hum_u.f = humidity;
	
	//add in temp and humidity (8 bytes)
	/*for (int i = 0; i < TEMP_ARR_SIZE * 2; i++) {
		if (i < 4) {
			if(temp_u.f == 99.99)
				arr[i] = 0;
			else
				arr[i] = temp_u.b[i];
		}
		else {
			//Serial.print("d.humb: "); Serial.println(d.hum_u.b[i]);
			if (hum_u.f == 99.99) {
				arr[i] = 0;
			}
			else {
				arr[i] = hum_u.b[i - 4];
			}
		}
	}
	*/
	return true;
}

//not used --> use payload::parseFloatF
bool DHTData::ParseTemp(Payload &p) {
	/*
	for (int i = 1; i < sizeof(temp_u.b) / sizeof(*temp_u.b) * 2 + 1; i++) {
		if (i < 5) {
			temp_u.b[i - 1] = p.message[i];
		}
		else {
			hum_u.b[i - 5] = p.message[i];
		}
	}
	*/

	return false;
}