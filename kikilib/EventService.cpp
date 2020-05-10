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

//���¼��������в���һ���¼�,�����̰߳�ȫ��
void EventService::insertEvInThisEvMgr(EventService* ev)
{
	_pMyEvMgr->insertEv(ev);
}

//���¼����������Ƴ�һ���¼�,�����̰߳�ȫ��
void EventService::removeEvInThisEvMgr(EventService* ev)
{
	_pMyEvMgr->removeEv(ev);
}

//���¼����������޸�һ���¼���������ע���¼�����,�����̰߳�ȫ��
void EventService::modifyEvInThisEvMgr(EventService* ev)
{
	_pMyEvMgr->modifyEv(ev);
}

//���EventManager����Ψһ������������
void* EventService::getThisEvMgrCtx()
{
	return _pMyEvMgr->getEvMgrCtx();
}

//�����¼����ʹ����¼�
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
	if (_eventState & (EPOLLIN | EPOLLPRI))
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

//дһ��int
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

//��ȡһ��int����������û�У��򷵻�false
bool EventService::readInt32(int& res)
{
	return _bufReader.readInt32(res);
}

std::string EventService::readBuf(size_t len)
{
	return _bufReader.read(len);
}

//��ȡ����Ϊlen�����ݣ���û�г���Ϊlen�����ݣ��򷵻�false
bool EventService::readBuf(char* buf, size_t len)
{
	return _bufReader.read(buf, len);
}

std::string EventService::readAll()
{
	return _bufReader.readAll();
}

//��һ�У�������\r\n��β,��û�У����ؿմ�
std::string EventService::readLineEndOfRN()
{
	return _bufReader.readLineEndOfRN();
}

//��һ�У�������\r��β,��û�У����ؿմ�
std::string EventService::readLineEndOfR()
{
	return _bufReader.readLineEndOfR();
}

//��һ�У�������\n��β,��û�У����ؿմ�
std::string EventService::readLineEndOfN()
{
	return _bufReader.readLineEndOfN();
}

TimerTaskId EventService::runAt(Time time, std::function<void()>&& timerCb)
{
	return _pMyEvMgr->runAt(time, std::move(timerCb));
}

TimerTaskId EventService::runAt(Time time, std::function<void()>& timerCb)
{
	return _pMyEvMgr->runAt(time, timerCb);
}

//timeʱ���ִ��timerCb����
TimerTaskId EventService::runAfter(Time time, std::function<void()>&& timerCb)
{ 
	return _pMyEvMgr->runAfter(time, std::move(timerCb));
}

//timeʱ���ִ��timerCb����
TimerTaskId EventService::runAfter(Time time, std::function<void()>& timerCb)
{
	return _pMyEvMgr->runAfter(time, timerCb);
}

//ÿ��timeʱ��ִ��timerCb����
void EventService::runEvery(Time time, std::function<void()>&& timerCb, TimerTaskId& retId)
{
	_pMyEvMgr->runEvery(time, timerCb, retId);
}

void EventService::runEvery(Time time, std::function<void()>& timerCb, TimerTaskId& retId)
{
	_pMyEvMgr->runEvery(time, timerCb, retId);
}

//ÿ��timeʱ��ִ��һ��timerCb����,ֱ��isContinue��������false
void EventService::runEveryUntil(Time time, std::function<void()>& timerCb, std::function<bool()>& isContinue, TimerTaskId& retId)
{
	_pMyEvMgr->runEveryUntil(time, timerCb, isContinue, retId);
}

void EventService::runEveryUntil(Time time, std::function<void()>& timerCb, std::function<bool()>&& isContinue, TimerTaskId& retId)
{
	_pMyEvMgr->runEveryUntil(time, timerCb, isContinue, retId);
}

void EventService::runEveryUntil(Time time, std::function<void()>&& timerCb, std::function<bool()>& isContinue, TimerTaskId& retId)
{
	_pMyEvMgr->runEveryUntil(time, timerCb, isContinue, retId);
}

void EventService::runEveryUntil(Time time, std::function<void()>&& timerCb, std::function<bool()>&& isContinue, TimerTaskId& retId)
{
	_pMyEvMgr->runEveryUntil(time, timerCb, isContinue, retId);
}

//���������Ѿ���ʱ����Ҫִ�еĺ���
void EventService::runExpired()
{
	_pMyEvMgr->runExpired();
}

//����������̳߳����Դﵽ�첽ִ�е�Ч��
void EventService::runInThreadPool(std::function<void()>&& func)
{
    return _pMyEvMgr->runInThreadPool(std::move(func));
}
