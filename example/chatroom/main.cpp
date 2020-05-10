//@Author Liu Yukang 
#include "ChatRoomService.h"
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
	std::string ip;
	int port = DecodeConfigFile();
	kikilib::EventMaster<ChatRoomService> evMaster;
	evMaster.init(1,port);
	evMaster.loop();
	return 0;
}