#include "GPS.h"

/*
#ifdef ARDUINO
#define printf Definitions::prf
#endif
*/

GPS::GPS(SoftwareSerial* _ss)
{
	ss = _ss;
}

void GPS::getCoords(float &lat, float &lon) {
	//get the nmea string
	unsigned long ms = 1000;
	unsigned long start = millis();
	do
	{
		while (ss->available())
			tgps.encode(ss->read());
	} while (millis() - start < ms);
	
	lat = tgps.location.lat();
	lon = tgps.location.lng();
}

void GPS::getCoords() {
	//get the nmea string
	unsigned long ms = 1000;
	unsigned long start = millis();
	do
	{
		while (ss->available())
			tgps.encode(ss->read());	//tgps.loaction.lat() updated here
	} while (millis() - start < ms);
}






/* Not used, use parsefloat in payload
void GPS::ParseGPS(Payload &p, uint8_t arr[]) {

	//uint8_t arr[sizeof(lat_u.b) * 2];	//temp and hum
	for (int i = 1; i < sizeof(arr) * 2 + 1; i++) {
		if (i < 9) {
			lat_u.b[i] = p.message[i];
		}
		else {
			lon_u.b[i - 8] = p.message[i];
		}
	}
}
*/

/* not used, use parsefloat in payload
void GPS::ConcatGPS(uint8_t arr[]) {
	//arr should be p.message array.
	Serial.print(lat_u.f, 8);
	const uint8_t sz = sizeof(lat_u.b) / sizeof(*lat_u.b);
	
	//add in temp and humidity (8 bytes)
	for (int i = 0; i < sz; i++) {
		if (i < 4) {
			if (lat_u.f == 99.99)
				arr[i] = 0;
			else
				arr[i] = lat_u.b[i];
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
	
	//memcpy(lat_u.b, arr + 1, sizeof(lat_u.b));	//index allows for 'G' at the start of message
	//memcpy(lon_u.b, arr + 5, sizeof(lon_u.b));

	

	for (int i = 0; i < sz; i++)
		Serial.print(lat_u.b[i]);
	Serial.println("..");

}
*/

void GPS::printFloat(float val)
{
	/*
	int len = 12;
	int prec = 6;

	if (!tgps.location.isValid())
	{
		printf("0\n");
	}
	else
	{
		printf("%f\n", val);
		int vi = abs((int)val);
		int flen = prec + (val < 0.0 ? 2 : 1); // . and -
		flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
		for (int i = flen; i<len; ++i)
			printf(" ");
	}
	*/
}