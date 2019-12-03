#include "KikitestServiceFactory.h"
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
	KikitestServiceFactory fac;
	int port = DecodeConfigFile();
	kikilib::EventMaster evMaster(&fac);
	evMaster.Loop(1, port);
	return 0;
}