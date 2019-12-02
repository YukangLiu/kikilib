#include "ChatRoomServiceFactory.h"
#include "EventMaster.h"

#include<string> 
#include<fstream>

std::set<ChatRoomService*> peersSet;

int DecodeConfigFile()
{
	std::ifstream infile("config.txt");
	std::string line;
	getline(infile, line);
	return atoi(line.c_str());
}

int main()
{
	ChatRoomServiceFactory fac;
	std::string ip;
	int port = DecodeConfigFile();
	kikilib::EventMaster evMaster(&fac);
	evMaster.Loop(1,port);
	return 0;
}