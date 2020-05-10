//@Author Liu Yukang 
#include "StaticHttpService.h"
#include "EventMaster.h"

int main()
{
	kikilib::EventMaster<StaticHttpService> evMaster;
	evMaster.Init(4,80);
	evMaster.Loop();
	return 0;
}