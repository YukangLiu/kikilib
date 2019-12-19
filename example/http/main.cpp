//@Author Liu Yukang 
#include "StaticHttpService.h"
#include "EventMaster.h"

int main()
{
	kikilib::EventMaster<StaticHttpService> evMaster;
	evMaster.init(4,80);
	evMaster.loop();
	return 0;
}