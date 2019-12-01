#include "StaticHttpServiceFactory.h"
#include "EventMaster.h"

#include<string> 
#include<fstream>

std::string DecodeConfigFile()
{
	std::ifstream infile("ip.txt");
	std::string line;
	getline(infile, line);
	return line;
}

int main()
{
	StaticHttpServiceFactory fac;
	std::string localIp = DecodeConfigFile();
	kikilib::EventMaster evMaster(&fac, localIp, 80);
	evMaster.Loop(3);
	return 0;
}