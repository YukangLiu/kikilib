#include "StaticHttpServiceFactory.h"
#include "EventMaster.h"

int main()
{
	StaticHttpServiceFactory fac;
	kikilib::EventMaster evMaster(&fac);
	evMaster.Loop(3,80);
	return 0;
}