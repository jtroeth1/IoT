#pragma once

#include "Definitions.h"
#include "Utilities.h"
#include "Data.h"

#ifndef ARDUINO
#include <sqlite3.h>
#include <sstream>
#endif

#include <string>

class SqlClass {
private:
	Logger _logger;

public:
	SqlClass();
	SqlClass(Logger* logger);
	//SqlClass(Data &_data);

	struct SqlData {
		int table_id;// = -1;
		int project_id;// = -1;
		int class_id; //= -1;
		int node_id;// = -1;
		double gps_lat;// = -1;
		double gps_lon;// = -1;
		float temp;
		long lastUpdateTime;
	};
	uint8_t tempCurrentNode;
	uint8_t gpsCurrentNode; //used to determine which node to request from
	SqlData sd[MAX_DEVICES]; //return data from query --> THIS SHOULD BE A LINKED LIST ELSE THIS IS HEAPS OF MEMORY (8KB)
	SqlData sdTrig;
	SqlData sdTrigVals;
	std::string buf;	//for custom sql query returns

	sqlite3* db;	//jora database

	char* createNodeTable();
	char* createGWTable();
	char* tableExists(std::string tableName);
	char* selectAll(std::string tableName);
	char* insertNewNode(uint8_t id[], float gps_lat, float gps_long);
	char* updateValue(uint8_t id[], int val, std::string col);
	char* updateValue(uint8_t id[], float val, std::string col);
	bool executeCommand(char* cmd);
	bool getNode(std::string custom = "");
	int nodeExists(uint8_t id[]);
	void getNodeCallback(int argc, char **argv, char **azColName);
	void clearSqlDataTrig();	//set the data in passed callback to current class data
	void clearSqlData();

	int setupNodes();

	//bool nodeExists_b;
	int node_total;	//total number of nodes used for table id
	uint8_t node_count;	//how many nodes the sql returns (no more than 255 = MAX_DEVICES)
	bool return_exempt;	//means its acceptable to return more than one result from id
	int result_ex;

private:
	void log(char* str);
	static int callback(void *param, int argc, char** argv, char **azColName);
	static int nodeExistsCallback(void *n, int argc, char** argv, char **azColName);
	static int getNodeQueryCallback(void *NotUsed, int argc, char** argv, char **azColName);

};

//extern SqlClass sql;