
#include "sql.h"
#include "Data.h"

SqlClass::SqlClass() {
	//nodeExists_b = false;
}

SqlClass::SqlClass(Logger* logger) {
	_logger = *logger;
}

 int SqlClass::callback(void *NotUsed, int argc, char** argv, char **azColName) {
	int i;
	
	Logger* l = (Logger*)malloc(sizeof(Logger*));// new Logger();
	l = new Logger();
	l->log(Definitions::appendDebug, "SQL: Returned --> ");
	for (i = 0; i < argc; i++) {
		char logStr[80];
		sprintf(logStr, "%s = %s, ", azColName[i], argv[i] ? argv[i] : "NULL");
		l->log(Definitions::appendDebug, logStr);
	}
	l->log(Definitions::executeDebug, "\n");
	free(l);

	return 0;
}

int SqlClass::nodeExistsCallback(void *data, int argc, char** argv, char **azColName) {
	 int i;
	 Logger* l = (Logger*)malloc(sizeof(Logger*));// new Logger();
	 l = new Logger();
	 int n = 0;

	 l->log(Definitions::appendDebug, "SQL: Returned --> ");
	 for (i = 0; i < argc; i++) {
		 //for logging only
		 char logStr[80];
		 sprintf(logStr, "%s = %s, ", azColName[i], argv[i] ? argv[i] : "NULL");
		 l->log(Definitions::appendDebug, logStr);

		 //return number of items (which is how many nodes exist)
		 //std::stringstream conv(argv[i]);
		 //conv >> n;
		 n++;
	 }
	 l->log(Definitions::executeDebug, "");	//log string
	 l->log(Definitions::debug, "SQL: Returned %d items\n", n);	//log to debug number of nodes?
	 
	 // convert n into *data and use that as exists count ********************
	 //printf("data addr: %p, n addr: %p\n", data, &n);
	 *(int *)data = n;
	 //printf("data in cb: %d (data addr: %p)\n", *(int *)data, data);

	 free(l);

	 return 0;
 }

 int SqlClass::nodeExists(uint8_t id[]) {
	 char* zErrMsg = 0;
	 int rc;
	 void* data = malloc(sizeof(int));

	 rc = sqlite3_open(DATABASE_NAME, &db);

	 if (rc) {
		 _logger.log("Can't open database: %s\n", sqlite3_errmsg(db));
		 return SQL_CANT_OPEN_ERR;
	 }
	 else {
		 _logger.log(Definitions::debug, "Opened database successfully\n");
	 }

	 //create command
	 std::string n = std::to_string(Utilities::ConvertToUint16_t(id[2], id[3]));
	 std::string query = "SELECT EXISTS(SELECT * FROM NODES WHERE NODES.NODE_ID = " + n + ")";

	 _logger.log(Definitions::debug, "SQL: Query --> %s", query.c_str());
	 
	 //execute cmd
	 rc = sqlite3_exec(db, query.c_str(), nodeExistsCallback, data, &zErrMsg);
	 
	 //printf("data addr: %p\n", data);

	 int nodesReturned = reinterpret_cast<int>(*(int*)data);
	 //printf("data: %d\n", (int)*data);
	 //printf("*nodesReturned: %d\n", nodesReturned);
	 free(data);


	 if (rc != SQLITE_OK) {
		 _logger.log("SQL: Error --> %s\n", zErrMsg);
		 sqlite3_free(zErrMsg);
		 return SQL_QUERY_ERR;
	 }
	 else {
		 _logger.log(Definitions::debug,  "SQL: Operation completed successfully\n");

		 if (nodesReturned > 1) {
			_logger.log("More than one node with this id exists!\n");
			return false;
		 }
		 else if (nodesReturned < 1) {
			 return NODE_ID_NOT_SET_ERR;
		 }

		 clearSqlDataTrig();
	 }
	 
	 sqlite3_close(db);
	 
	 return true;
 }

 bool SqlClass::executeCommand(char* cmd) {
	 char* zErrMsg = 0;
	 int rc;
	 const char* data = "Callback function called";
	 
	 rc = sqlite3_open(DATABASE_NAME, &db);

	 if (rc) {
		 _logger.log("SQL: Can't open database: %s\n", sqlite3_errmsg(db));
		 return false;
	 }
	 else {
		 _logger.log(Definitions::debug, "SQL: Opened database successfully\n");
	 }

	 //execute cmd
	 rc = sqlite3_exec(db, cmd, callback, (void*)data, &zErrMsg);
	 
	 if (rc != SQLITE_OK) {
		 _logger.log("SQL: Error --> %s\n", zErrMsg);
		 sqlite3_free(zErrMsg);
		 return false;
	 }
	 else {
		 _logger.log(Definitions::debug, "SQL: Operation completed successfully\n");
		 clearSqlDataTrig();
	 }
	 sqlite3_close(db);
	 
	 return true;
	 
}

 //pass to public func
 int SqlClass::getNodeQueryCallback(void *param, int argc, char** argv, char **azColName) {
	 
	 SqlClass *d = reinterpret_cast<SqlClass*>(param);
	 d->getNodeCallback(argc, argv, azColName);
	 //printf("inQueryCb tid: %d, pid: %d, cid: %d, nid: %d, lat: %f, lon: %f\n", d->sd.table_id, d->sd.project_id, d->sd.class_id, d->sd.node_id, d->sd.gps_lat, d->sd.gps_lon);
	 d->_logger.log(Definitions::executeDebug, "");
	 
	 return 0;
 }

 //clears query trigger data
 void SqlClass::clearSqlDataTrig() {
	 sdTrig.table_id = 0; sdTrigVals.table_id = 0;
	 sdTrig.project_id = 0; sdTrigVals.project_id = 0;
	 sdTrig.class_id = 0; sdTrigVals.class_id = 0;
	 sdTrig.node_id = 0; sdTrigVals.node_id = 0;
	 sdTrig.gps_lat = 0; sdTrigVals.gps_lat = 0;
	 sdTrig.gps_lon = 0; sdTrigVals.gps_lon = 0;
	 sdTrig.temp = 0; sdTrigVals.temp = 0;
	 sdTrig.lastUpdateTime = 0; sdTrigVals.lastUpdateTime = 0;
 }

 //Clear data in the array lelz
 void SqlClass::clearSqlData() {
	 for (int i = 0; i < MAX_DEVICES; i++) {
		 sd[i].table_id = 0;
		 sd[i].project_id = 0;
		 sd[i].class_id = 0;
		 sd[i].node_id = 0;
		 sd[i].gps_lat = 0;
		 sd[i].gps_lon = 0;
		 sd[i].temp = 0;
		 sd[i].lastUpdateTime = 0;
	 }
 }

 //can deal with values in here
void SqlClass::getNodeCallback(int argc, char **argv, char **azColName)
 {

	 _logger.log(Definitions::appendDebug, "SQL Returned: ");
	 for (int i = 0; i < argc; i++) {
		 _logger.log(Definitions::appendDebug, "%s = %s, ", azColName[i], argv[i] ? argv[i] : "NULL");
		// std::stringstream ss(argv[i] ? argv[i] : "NULL");
		 
		 if (argv[i] != NULL) {
			 if (azColName[i] == (std::string)TABLE_ID_S) {
				 sd[node_count].table_id = std::stoi(argv[i]);
			 }
			 else if (azColName[i] == (std::string)PROJECT_ID_S) {
				 sd[node_count].project_id = std::stoi(argv[i]);
			 }
			 else if (azColName[i] == (std::string)CLASS_ID_S) {
				 sd[node_count].class_id = std::stoi(argv[i]);
			 }else if (azColName[i] == (std::string)NODE_ID_S) {
				 sd[node_count].node_id = std::stoi(argv[i]);
			 }else if (azColName[i] == (std::string)GPS_LAT_S) {
				 sd[node_count].gps_lat = std::stod(argv[i]);			 
			 }else if (azColName[i] == (std::string)GPS_LON_S) {
				 sd[node_count].gps_lon = std::stod(argv[i]);
			 }else if (azColName[i] == (std::string)TEMP_S) {
				 sd[node_count].temp = std::stof(argv[i]);
			 }else if (azColName[i] == (std::string)LAST_UPDATE_TIME_S) {
				 sd[node_count].lastUpdateTime = std::stoi(argv[i]);
			 }else {
				 buf = argv[i];
				 //_logger.log("argv: %s buf: %s", argv[i], buf);
			 }
				 
		 }
		 
	 }
	 
	 _logger.log(Definitions::appendDebug, "\n");
	 node_count++;	//increment node_count here else it will add data starting at 1st index of sd array

	 if (!return_exempt && node_count > 1) {
		 _logger.log(Definitions::debug,  "SQL: Query returned more than 1 result!\n");
		 result_ex = 1;
	 }
	 result_ex = 0;

	 //printf 10 of the best
	 /*for (int i = 0; i < 10; i++) {
		 printf("tid: %d, pid: %d, cid: %d, nid: %d, lat: %f, lon: %f\n", sd[i].table_id, sd[i].project_id, sd[i].class_id, sd[i].node_id, sd[i].gps_lat, sd[i].gps_lon);
	 }*/
	 
 }

int SqlClass::setupNodes() {
	buf == "";
	std::string q = "SELECT MAX(" + (std::string)NODE_ID_S + ") FROM " + (std::string)NODE_TABLE + " WHERE " + (std::string)CLASS_ID_S + " = " + std::to_string(SLAVE_CLASS_ID);
	char* cmd = new char[q.length() + 1];
	strcpy(cmd, q.c_str());
	getNode(cmd);
	int total = 0;
	if (node_count > 0) {	//if at least one slave was returned then try parse to int
		try {
			if (buf != "") {
				//_logger.log("buf: %s, node count: %d", node.sql.buf.c_str(), node.sql.node_count);				
				total = std::stoi(buf);
			}
			else {
				total = 0;
			}
		}
		catch (...) {
			_logger.log("stoi conversion failed!");
		}
	}

	return total;
}

 bool SqlClass::getNode(std::string custom) {
	 char* zErrMsg = 0;
	 int rc;

	 rc = sqlite3_open(DATABASE_NAME, &db);

	 if (rc) {
		 _logger.log("SQL: Can't open database: %s\n", sqlite3_errmsg(db));
		 return false;
	 }
	 else {
		 _logger.log(Definitions::debug, "SQL: Opened database successfully\n");
	 }

	 //create command
	 std::string query;
	 if (custom == "") {
		 //check primarily based on node id
		 query = "SELECT * FROM " + (std::string)NODE_TABLE + " WHERE " + (std::string)NODE_TABLE + ".";
		 if (sdTrig.node_id > 0) {
			 if (sdTrig.class_id) 	//if just wanting a node from a class
				 query += (std::string)CLASS_ID_S + " = " + std::to_string(sdTrigVals.class_id) + " AND ";
			 query += (std::string)NODE_ID_S + " = " + std::to_string(sdTrigVals.node_id);
		 }
		 else if (sdTrig.class_id >= 0) {
			 query += (std::string)CLASS_ID_S + " = " + std::to_string(sdTrigVals.class_id);
			 return_exempt = true;
		 }
		 else if (sdTrig.project_id >= 0) {
			 query += (std::string)PROJECT_ID_S + " = " + std::to_string(sdTrigVals.project_id);
			 return_exempt = true;
		 }
		 else if (sdTrig.gps_lat > 0) {
			 query += (std::string)GPS_LAT_S + " = " + std::to_string(sdTrigVals.gps_lat);
			 return_exempt = true;
		 }
		 else if (sdTrig.gps_lon > 0) {
			 query += (std::string)GPS_LON_S + " = " + std::to_string(sdTrigVals.gps_lon);
			 return_exempt = true;
		 }
		 else if (sdTrig.temp > 0) {
			 query += (std::string)TEMP_S + " = " + std::to_string(sdTrigVals.temp);
			 return_exempt = true;
		 }
		 else if (sdTrig.lastUpdateTime > 0) {
			 query += (std::string)LAST_UPDATE_TIME_S + " = " + std::to_string(sdTrigVals.lastUpdateTime);
			 return_exempt = true;
		 }
	 }
	 else {
		 query = custom;
	 }
	 
	 //execute cmd
	 node_count = 0;	//reset
	 _logger.log(Definitions::debug, "SQL: Query --> %s\n", query.c_str());
	 
	 rc = sqlite3_exec(db, query.c_str(), getNodeQueryCallback, this, &zErrMsg);

	 if (rc != SQLITE_OK) {
		 _logger.log("SQL: error --> %s\n", zErrMsg);
		 sqlite3_free(zErrMsg);
		 return false;
	 }
	 else {
		 _logger.log(Definitions::debug, "SQL: Operation completed successfully\n");
	 }
	 sqlite3_close(db);

	 return_exempt = false;
	 clearSqlDataTrig();
	 //printf("node id: %d\n", sd[0].node_id);
	 return true;

 }

 char* SqlClass::insertNewNode(uint8_t id[], float gps_lat, float gps_lon) {
	 
	 node_total++;	//for table_id
	 _logger.log("node total: %d\n", node_total);
	 std::string tid = std::to_string(node_total);
	 std::string pid = std::to_string(id[0]);
	 std::string cid = std::to_string(id[1]);
	 std::string n = std::to_string(Utilities::ConvertToUint16_t(id[2], id[3]));
	 std::string lat = std::to_string(gps_lat);
	 std::string lon = std::to_string(gps_lon);

	 std::string query = "INSERT INTO " + (std::string)NODE_TABLE + "(" + (std::string)TABLE_ID_S + ", " + (std::string)PROJECT_ID_S + ", "\
		 + (std::string)CLASS_ID_S + ", " + (std::string)NODE_ID_S + ", " + (std::string)GPS_LAT_S + ", " + (std::string)GPS_LON_S + ", "\
		 + (std::string)TEMP_S + ", " + (std::string)LAST_UPDATE_TIME_S + ")"\
		 "VALUES (" + tid + "," + pid + "," + cid + "," + n + "," \
		 + lat + "," + lon + ", 0" + ", 0" + "); ";	//temp and update time = 0

	 _logger.log(Definitions::debug, "SQL: Inserting --> %s, %s, %s, %s, %s, %s, %s, %s\n", tid.c_str(), pid.c_str(), cid.c_str(), n.c_str(), lat.c_str(), lon.c_str(), "0", "0");
	 //printf("%s", query.c_str());
	 //some magic to convert to char*
	 char* cmd = new char[query.length() + 1];
	 strcpy(cmd, query.c_str());

	 return cmd;
}

 //returns a query for execute command
 char* SqlClass::updateValue(uint8_t id[], int val, std::string col) {
	 std::string idStr = std::to_string(Utilities::ConvertToUint16_t(id[2], id[3]));
	 std::string v = std::to_string(val);

	 std::string query = "UPDATE " + (std::string)NODE_TABLE + " SET " + col + " = " + v + " WHERE " + (std::string)NODE_ID_S + " = " + idStr;
	 char* cmd = new char[query.length() + 1];
	 strcpy(cmd, query.c_str());

	 return cmd;
 }

 //overload to above func
 char* SqlClass::updateValue(uint8_t id[], float val, std::string col) {
	 std::string idStr = std::to_string(Utilities::ConvertToUint16_t(id[2], id[3]));
	 std::string v = std::to_string(val);

	 std::string query = "UPDATE " + (std::string)NODE_TABLE + " SET " + col + " = " + v + " WHERE " + (std::string)NODE_ID_S + " = " + idStr;
	 char* cmd = new char[query.length() + 1];
	 strcpy(cmd, query.c_str());

	 return cmd;
 }

 //static bool basedOnLocation;	//means its acceptable to return more than one result from id
 char* SqlClass::tableExists(std::string tableName) {
	
	 std::string q = "SELECT * FROM sqlite_master WHERE name LIKE " + tableName;
	 //char* cmd = "SELECT * FROM sysobjects WHERE id = object_id(N'COMPANY')";
	 char* cmd = new char[q.length() + 1];
	 strcpy(cmd, q.c_str());

	 return cmd;
}

 char* SqlClass::selectAll(std::string tableName) {
	std::string c = "SELECT * FROM " + tableName;
	char* cmd = new char[c.length() + 1];
	strcpy(cmd, c.c_str());

	return cmd;
}

char* SqlClass::createNodeTable() {
	char* cmd = "CREATE TABLE NODES("  \
		"TABLE_ID INT PRIMARY KEY     NOT NULL," \
		"PROJECT_ID		INT		      NOT NULL," \
		"CLASS_ID		INT		      NOT NULL," \
		"NODE_ID		INT		      NOT NULL," \
		"GPS_LAT		REAL," \
		"GPS_LON         REAL," \ 
		"TEMP			REAL," \
		"LAST_UPDATE_TIME		INT		NOT NULL); ";
	_logger.log("Creating Node table...\n");

	return cmd;
}

char* SqlClass::createGWTable() {
	char* cmd = "CREATE TABLE GW("  \
		"TABLE_ID INT PRIMARY KEY     NOT NULL," \
		"ACTIVE_NODES		INT		      NOT NULL," \
		"TOTAL_NODES		INT		      NOT NULL," \
		"LAST_SENSOR_UPDATE_TIME		INT		      NOT NULL," \
		"LAST_DOWNLINK_UPDATE_TIME		INT		      NOT NULL); ";
	
	_logger.log("Creating Gateway table...\n");

	return cmd;
}

//SqlClass sql;