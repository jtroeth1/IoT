#pragma once

#include <string.h>

//#define DEBUG	//define to enable extra debug printing
#define SIMULATE	//uncomment to fake peripheral data
#define PINGTEST

//has to be master if not arduino
#ifndef ARDUINO
#define MASTER
#else
////#define printf	prf
#endif

//#define UNTESTED
#define ID_ADDR	100	//store first id here then 3 following are used
#define PROJECT_ID	0	//0 = prototype

#define MASTER_CLASS_ID	100
#define SLAVE_CLASS_ID	200

#ifdef MASTER
#define CLASS_ID	MASTER_CLASS_ID		//master
#else
#define CLASS_ID	SLAVE_CLASS_ID	//slave
#endif

#define MASTER_NODE_ID	1	//= 0.0.0.1
#define BROADCAST_ID	0
#define RX_SS_PIN	9	//8,9,10,11,14,15,16 only
#define TX_SS_PIN	8
#define ID_NOT_SET	255
#define	ID_NOT_SET16	65535
#define TX_POWER	3
#define MAX_RESEND_RETRIES	4
#define RESEND_LIMIT	4

#define MAX_DEVICES	255//This significantly effects memory usage for pro mini
#define ID_SIZE	4
#define MAX_MESSAGE_LENGTH	30
#define MESSAGE_IN_BUF_SIZE	200
#define MESSAGE_CHAR_SIZE	3
#define MAX_MESSAGE_LENGTH_CHAR	10
#define PAYLOAD_SIZE	60
#define CONVERT_CHAR_INT	48
#define FLOAT_BYTE_UNION_SIZE	4
#define SIZE_BYTE	255
#define MAX_BYTE_SIZE	9

#define NODE_SENSOR_UPDATE_TIME	1	//minutes to update data from nodes about sensors
#define TIME_NOT_SET	70	//year as in 1970 = first epoch time means it hasnt been set
#define CHECK_DOWNLINK_TIME	1	//time to check downlinks seconds
#define MIN_SEND_TIME	1	//minium between message sending secs

//sql
#define	DATABASE_NAME	"/home/master/IOT/Jora/sql/jora_database.db"
#define NODE_TABLE	"NODES"
#define GW_TABLE	"GW"
#define NODE_COUNT_EX	1
#define TABLE_ID_S	"TABLE_ID"	//make sure to cast as std::string
#define PROJECT_ID_S	"PROJECT_ID"
#define CLASS_ID_S	"CLASS_ID"
#define NODE_ID_S	"NODE_ID"
#define GPS_LAT_S	"GPS_LAT"
#define GPS_LON_S	"GPS_LON"
#define TEMP_S	"TEMP"
#define LAST_UPDATE_TIME_S	"LAST_UPDATE_TIME"
#define MULTIPLE_NODES_EXIST_ERR	78
#define NODE_ID_NOT_SET_ERR	17
#define SQL_QUERY_ERR	0
#define SQL_CANT_OPEN_ERR	18
//const std::string PROJECT_ID_S = "PROJECT_ID";

//file rw
#define DOWNLINK_FILENAME	"/home/master/IOT/Jora/Jora_server/Downlink.txt"
#define	LOG_FILENAME	"/home/master/IOT/Jora/log/Jora_gateway.log"
#define MAX_FILE_SIZE	1000000	//1MB
//Not used #define DEBUG_LOG_FILENAME "/home/master/IoT/Jora/log/Debug_Jora_gateway.log"

//sensors
#define DATA_UNION_LEN	4	//union is x bytes
//Temperature
#define DHTPIN 3
#define DHTTYPE DHT11 
#define TEMP_INVALID	99.99

//GPS
#define GPS_TX	8	//goes to rx on gps
#define GPS_RX	9
#define GPS_BAUD	9600

//LED
#define LED_PIN	6

class Definitions
{
public : enum cmd {
		ASSIGN_ID = 'A',
		HANDSHAKE = 'H',
		LED = 'L',
		TEMP = 'T',
		NI = '$',
		BP = '#',	//blank placeholder
		ACK = 6,	//corresponds to ascii 'ACK'
		SQL = 'S',	//for sql queries
		GPS = 'G',	//GPS cmd
		CMD = '@',	//following is a cmd (only used for downlink messages at this stage)
		DOT = '.',
		PING = 'P',	//Ping test
		PONG = 'N'
	};

public: enum printTypes {
	log,
	debug,
	append,
	execute,
	appendDebug,
	executeDebug
};

public: enum logType {
	eventLogger,	//logs like text based logger
	dataLogger	//logs like csv debug data
};

public:
	Definitions();

	static void prf(char* fmt, ...);
};

//printing
/*
This code should be pasted within the files where this function is needed.
This function will not create any code conflicts.

The function call is similar to printf: ardprintf("Test %d %s", 25, "string");
To print the '%' character, use '%%'
This code was first posted on http://arduino.stackexchange.com/a/201
*/

/*
#ifndef ARDPRINTF
#endif
#ifdef ARDUINO
#include <stdarg.h>
#include <Arduino.h> //To allow function to run from any file in a project

#define ARDBUFFER	64	//effects memory

int ardprintf(char *str, ...) //Variadic Function
{
int i, count = 0, j = 0, flag = 0;
char temp[ARDBUFFER + 1];
for (i = 0; str[i] != '\0'; i++)  if (str[i] == '%')  count++; //Evaluate number of arguments required to be printed

va_list argv;
va_start(argv, count);
for (i = 0, j = 0; str[i] != '\0'; i++) //Iterate over formatting string
{
if (str[i] == '%')
{
//Clear buffer
temp[j] = '\0';
Serial.print(temp);
j = 0;
temp[0] = '\0';

//Process argument
switch (str[++i])
{
case 'd': Serial.print(va_arg(argv, int));
break;
case 'l': Serial.print(va_arg(argv, long));
break;
case 'f': Serial.print(va_arg(argv, double));
break;
case 'c': Serial.print((char)va_arg(argv, int));
break;
case 's': Serial.print(va_arg(argv, char *));
break;
default:;
};
}
else
{
//Add to buffer
temp[j] = str[i];
j = (j + 1) % ARDBUFFER;
if (j == 0)  //If buffer is full, empty buffer.
{
temp[ARDBUFFER] = '\0';
Serial.print(temp);
temp[0] = '\0';
}
}
};

Serial.println(); //Print trailing newline
return count + 1; //Return number of arguments detected
}
#endif
#endif
*/

