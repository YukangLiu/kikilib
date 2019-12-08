//@Author Liu Yukang 
#include "StaticHttpService.h"
#include "EventMaster.h"

int main()
{
	kikilib::EventMaster<StaticHttpService> evMaster;
	evMaster.Loop(4,80);
	return 0;
}