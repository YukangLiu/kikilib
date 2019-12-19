//@Author Liu Yukang 
#include "echoService.h"

#include <string>

void EchoService::handleReadEvent()
{
	std::string str = readAll();
	sendContent(std::move(str));
	forceClose();
};

void EchoService::handleErrEvent()
{
	forceClose();
};