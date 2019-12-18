//@Author Liu Yukang 
#include "echoService.h"
#include "EventMaster.h"

int main()
{
	kikilib::EventMaster<EchoService> evMaster;
	evMaster.init(4, 80);
	evMaster.loop();
	return 0;
}