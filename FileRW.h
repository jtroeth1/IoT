#pragma once

#include "Definitions.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <libgen.h>

#ifndef ARDUINO	//arduino wouldnt be defined... meh
#include "sys/stat.h"
#include <boost/filesystem.hpp>
#endif

class FileRW
{
public:
	FileRW();
	~FileRW();
	
	bool GetDownlinkMessage(std::string fileName, std::string &d, char dMess[]);
	bool append(std::string str, std::string dir);
	std::vector<std::string> split(const std::string s);	//split a string
	bool CheckHeader(std::string dir);
	void ArchiveLogFile(std::string fileName);

private:
	//static void readLine(std::string fileName);
	void rmLine(std::string fileName);
};

