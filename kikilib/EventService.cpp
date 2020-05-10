//@Author Liu Yukang 
#include "EventService.h"
#include "EventManager.h"
#include "LogManager.h"

#include <poll.h>
#include <sys/epoll.h>

using namespace kikilib;

EventService::EventService(Socket& sock, EventManager* evMgr, int interestEvent)
	: _interestEvent(interestEvent), _eventState(0), _eventPriority(NORMAL_EVENT),
	_isConnected(true), _sock(sock), _pMyEvMgr(evMgr), _bufWritter(sock, this), _bufReader(sock)
{}

EventService::EventService(Socket&& sock, EventManager* evMgr, int interestEvent)
	: _interestEvent(interestEvent), _eventState(0), _eventPriority(NORMAL_EVENT),
	_isConnected(true), _sock(sock), _pMyEvMgr(evMgr), _bufWritter(sock, this), _bufReader(sock)
{}

void EventService::forceClose()
{
    if(_isConnected)
    {
        _isConnected = false;
        handleCloseEvent();
        _pMyEvMgr->removeEv(this);
    }

}

void EventService::shutDownWrite()
{
	_sock.shutdownWrite();
}

void EventService::setInteresEv(int newInterestEv)
{
	_interestEvent = newInterestEv;
	_pMyEvMgr->modifyEv(this);
}

//向事件管理器中插入一个事件,这是线程安全的
void EventService::insertEvInThisEvMgr(EventService* ev)
{
	_pMyEvMgr->insertEv(ev);
}

//向事件管理器中移除一个事件,这是线程安全的
void EventService::removeEvInThisEvMgr(EventService* ev)
{
	_pMyEvMgr->removeEv(ev);
}

//向事件管理器中修改一个事件服务所关注的事件类型,这是线程安全的
void EventService::modifyEvInThisEvMgr(EventService* ev)
{
	_pMyEvMgr->modifyEv(ev);
}

//获得EventManager区域唯一的上下文内容
void* EventService::getThisEvMgrCtx()
{
	return _pMyEvMgr->getEvMgrCtx();
}

//根据事件类型处理事件
void EventService::handleEvent()
{
	if ((_eventState & EPOLLHUP) && !(_eventState & EPOLLIN))
	{
		forceClose();
	}

	if (_eventState & (EPOLLERR))
	{
        RecordLog(ERROR_DATA_INFORMATION, "error event in HandleEvent()!");
        handleErrEvent();
        forceClose();
	}
	if ((_eventState & (EPOLLIN | EPOLLPRI)))
	{
		if (!_bufReader.isEmptyAfterRead())
		{
			handleReadEvent();
		}
		else
		{
			forceClose();
		}
		/*else
		{
			RecordLog(ERROR_DATA_INFORMATION, "read fd error!");
			HandleErrEvent();
		}*/
	}
	if (_eventState & EPOLLOUT)
	{
		handleWriteEvent();
	}
}

void EventService::handleWriteEvent()
{
	_bufWritter.writeBufToSock();
}

//写一个int
bool EventService::sendInt32(int num)
{
	return _bufWritter.sendInt32(num);
}

bool EventService::sendContent(std::string& content)
{
	return _bufWritter.send(content);
}

bool EventService::sendContent(std::string&& content)
{
	return _bufWritter.send(std::move(content));
}

//读取一个int，若缓存中没有，则返回false
bool EventService::readInt32(int& res)
{
	return _bufReader.readInt32(res);
}

std::string EventService::readBuf(size_t len)
{
	return _bufReader.read(len);
}

//读取长度为len的数据，若没有长度为len的数据，则返回false
bool EventService::readBuf(char* buf, size_t len)
{
	return _bufReader.read(buf, len);
}

std::string EventService::readAll()
{
	return _bufReader.readAll();
}

//读一行，该行以\r\n结尾,若没有，返回空串
std::string EventService::readLineEndOfRN()
{
	return _bufReader.readLineEndOfRN();
}

//读一行，该行以\r结尾,若没有，返回空串
std::string EventService::readLineEndOfR()
{
	return _bufReader.readLineEndOfR();
}

//读一行，该行以\n结尾,若没有，返回空串
std::string EventService::readLineEndOfN()
{
	return _bufReader.readLineEndOfN();
}

void EventService::runAt(Time time, std::function<void()>&& timerCb)
{
	_pMyEvMgr->runAt(time, std::move(timerCb));
}

void EventService::runAt(Time time, std::function<void()>& timerCb)
{
	_pMyEvMgr->runAt(time, timerCb);
}

//time时间后执行timerCb函数
void EventService::runAfter(Time time, std::function<void()>&& timerCb)
{ 
	_pMyEvMgr->runAfter(time, std::move(timerCb));
}

//time时间后执行timerCb函数
void EventService::runAfter(Time time, std::function<void()>& timerCb)
{
	_pMyEvMgr->runAfter(time, timerCb);
}

//每过time时间执行timerCb函数
void EventService::runEvery(Time time, std::function<void()>&& timerCb)
{
	_pMyEvMgr->runEvery(time, timerCb);
}

void EventService::runEvery(Time time, std::function<void()>& timerCb)
{
	_pMyEvMgr->runEvery(time, timerCb);
}

//每过time时间执行一次timerCb函数,直到isContinue函数返回false
void EventService::runEveryUntil(Time time, std::function<void()>& timerCb, std::function<bool()>& isContinue)
{
	_pMyEvMgr->runEveryUntil(time, timerCb, isContinue);
}

void EventService::runEveryUntil(Time time, std::function<void()>& timerCb, std::function<bool()>&& isContinue)
{
	_pMyEvMgr->runEveryUntil(time, timerCb, isContinue);
}

void EventService::runEveryUntil(Time time, std::function<void()>&& timerCb, std::function<bool()>& isContinue)
{
	_pMyEvMgr->runEveryUntil(time, timerCb, isContinue);
}

void EventService::runEveryUntil(Time time, std::function<void()>&& timerCb, std::function<bool()>&& isContinue)
{
	_pMyEvMgr->runEveryUntil(time, timerCb, isContinue);
}

//运行所有已经超时的需要执行的函数
void EventService::runExpired()
{
	_pMyEvMgr->runExpired();
}

//将任务放在线程池中以达到异步执行的效果
void EventService::runInThreadPool(std::function<void()>&& func)
{
    return _pMyEvMgr->runInThreadPool(std::move(func));
}
