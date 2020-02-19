#include "FileRW.h"
#include "boost/algorithm/string.hpp"

FileRW::FileRW()
{
}


FileRW::~FileRW()
{
}

//used to check if data loffer file has a header or not
bool FileRW::CheckHeader(std::string dir) {
	std::ifstream file;
	file.open(dir);


	std::string line;
	bool atLeastOneLine = false;
	if (file.is_open()) {
		while (std::getline(file, line)) {	//while there is a line available
			atLeastOneLine = true;
			break;
		}
	}

	return atLeastOneLine;
}

void FileRW::ArchiveLogFile(std::string fileName) {
	//get time
	time_t timeNow = time(0);
	tm *ltm = localtime(&timeNow);

	std::vector<std::string> dirs = split(fileName);
	std::string name = dirs[dirs.size() - 1];

	std::string fullDir;
	for (uint8_t i = 0; i < dirs.size() - 1; i++) {
		if (dirs[i] == name)
			break;
		if (dirs[i] != "")
			fullDir += '/' + dirs[i];
	}
	fullDir += "/archive";	//add archive folder to dir
	//printf("%s\n", fullDir.c_str());

	//Check if the archive folder exists
	struct stat info;
	boost::filesystem::path p{ fullDir };
	if (stat(fullDir.c_str(), &info) != 0)
		boost::filesystem::create_directory(p);

	//append the current time to the log file name
	char t[80];
	strftime(t, 80, "%Y_%m_%d__%H:%M:%S", ltm);
	std::string time(t);
	std::string newLogFileName = (fullDir + "/" + time + "__" + name).c_str();
	//printf("%s\n", newLogFileName.c_str());

	std::rename(fileName.c_str(), newLogFileName.c_str());	//rename to archive folder
}

bool FileRW::append(std::string str, std::string dir) {
	std::ofstream file;
	file.open(dir, std::ios_base::app);
	
	if (file.is_open()) {
		file << str;	//copy str into new file
		file.close();	//close new file
		
		return true;
	}
	else {
		//printf("Failed to open file!");
		return false;
	}
}

std::vector<std::string> FileRW::split(const std::string s) {
	std::vector<std::string> results;
	boost::split(results, s, [](char c) {return c == '/'; });
	return results;
}

/*
void FileRW::readLine(std::string fileName, Data &d) {
	//open file in read mode
	std::ifstream file;
	std::string line;
	file.open(fileName);

	//read in a line
	std::vector<std::string> l;
	if (file.is_open()) {
		while (std::getline(file, line)) {	//while there is a line available (but we'll break after reading first line)
			l.push_back(line);
			//for (size_t i = 0; i < l.size(); i++)
			//	d.cMessage[i] = l[i];
			//break;
		}
	}

	//close file
	file.close();
}
*/

void FileRW::rmLine(std::string fileName) {
	return;
}

bool FileRW::GetDownlinkMessage(std::string fileName, std::string &d, char dMess[]) {
	
	//open files
	std::ifstream inFile;	//read 
	std::ofstream tempFile;	//write
	
	//get dir and filename
	char* ts1 = strdup(fileName.c_str());	
	char* dir = dirname(ts1);
	//char* filename = basename(ts2);

	//_logger.log("dir: %s, %s\n", dir, filename);
	std::string tempFileName = "/temp.txt";

	//open files
	inFile.open(fileName);

	//read in a line
	//std::vector<std::string> l;
	std::string linesForNewFile;
	std::string line;
	bool available = false;
	int lineCount = 0;	//only used for first line
	if (inFile.is_open()) {
		while (std::getline(inFile, line)) {	//while there is a line available
			//_logger.log("line: %s\n", line.c_str());
			
			available = true;	//at least one line is available
			if (lineCount < 1) {	//get the first line and copy into cMessage
				//l.push_back(line);
				lineCount++;
				d = line;
				for (size_t i = 0; i < line.size(); i++) {

					//_logger.log("l: %s\n", l[i].c_str());
					dMess[i] = line[i];
					if (line[i] == Definitions::BP)
						break;
					//_logger.log("cm: %c\n", d.cMessage[i]);
				}
			}
			else {	//add all but first line to the string
				linesForNewFile += line + "\n";
			}
		}
		linesForNewFile.erase(linesForNewFile.end() - 1);	//erase last char ******** do we need this??
		//_logger.log("for temp: %s\n", linesForNewFile.c_str());
	}
	inFile.close();	//close inFile

	//create temp file
	std::string dd = dir + tempFileName;
	tempFile.open(dd);
	//_logger.log("dir: %s\n", (dir + tempFileName).c_str());
	if (tempFile.is_open()) {
		//_logger.log("Writing new lines...\n");
		tempFile << linesForNewFile;	//copy str into new file
		tempFile.close();	//close new file
	}
	else {
		return false;
	}
	
	std::rename(dd.c_str(), fileName.c_str());	//rename new file to old files name (erases old file)
	
	return available;
}