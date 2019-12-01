#include "ChatRoomServiceFactory.h"
#include "EventMaster.h"

std::set<ChatRoomService*> peersSet;

int main()
{
	ChatRoomServiceFactory fac;

	kikilib::EventMaster evMaster(&fac, "192.168.206.128", 12345);
	evMaster.Loop(1);
	return 0;
}