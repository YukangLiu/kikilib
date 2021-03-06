//@Author Liu Yukang 
#include "KikitestService.h"
#include "EventMaster.h"

#include<string> 
#include<fstream>

int DecodeConfigFile()
{
	std::ifstream infile("config.txt");
	std::string line;
	getline(infile, line);
	return atoi(line.c_str());
}

int main()
{
	int port = DecodeConfigFile();
	kikilib::EventMaster<KikitestService> evMaster;
	evMaster.init(1, port);
	evMaster.loop();
	return 0;
}