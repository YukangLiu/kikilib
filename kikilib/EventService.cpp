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

void EventService::Close()
{
    if(_isConnected)
    {
        _isConnected = false;
        HandleCloseEvent();
        _pMyEvMgr->Remove(this);
    }

}

void EventService::ShutDownWrite()
{
	_sock.ShutdownWrite();
}

void EventService::SetInteresEv(int newInterestEv)
{
	_interestEvent = newInterestEv;
	_pMyEvMgr->Motify(this);
}

//向事件管理器中插入一个事件,这是线程安全的
void EventService::Insert(EventService* ev)
{
	_pMyEvMgr->Insert(ev);
}

//向事件管理器中移除一个事件,这是线程安全的
void EventService::Remove(EventService* ev)
{
	_pMyEvMgr->Remove(ev);
}

//向事件管理器中修改一个事件服务所关注的事件类型,这是线程安全的
void EventService::Motify(EventService* ev)
{
	_pMyEvMgr->Motify(ev);
}

//获得EventManager区域唯一的上下文内容
void* EventService::GetEvMgrCtx()
{
	return _pMyEvMgr->GetEvMgrCtx();
}

//根据事件类型处理事件
void EventService::HandleEvent()
{
	if ((_eventState & EPOLLHUP) && !(_eventState & EPOLLIN))
	{
		Close();
	}

	if (_eventState & (EPOLLERR))
	{
        RecordLog(ERROR_DATA_INFORMATION, "error event in HandleEvent()!");
        HandleErrEvent();
        Close();
	}
	if (_eventState & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
	{
		if (!_bufReader.IsEmptyAfterRead())
		{
			HandleReadEvent();
		}
		else
		{
			Close();
		}
		/*else
		{
			RecordLog(ERROR_DATA_INFORMATION, "read fd error!");
			HandleErrEvent();
		}*/
	}
	if (_eventState & EPOLLOUT)
	{
		HandleWriteEvent();
	}
}

void EventService::HandleWriteEvent()
{
	_bufWritter.WriteBufToSock();
}

//写一个int
void EventService::WriteInt32(int num)
{
	_bufWritter.SendInt32(num);
}

void EventService::WriteBuf(std::string& content)
{
	_bufWritter.Send(content);
}

void EventService::WriteBuf(std::string&& content)
{
	_bufWritter.Send(std::move(content));
}

//读取一个int，若缓存中没有，则返回false
bool EventService::ReadInt32(int& res)
{
	return _bufReader.ReadInt32(res);
}

std::string EventService::ReadBuf(size_t len)
{
	return _bufReader.Read(len);
}

//读取长度为len的数据，若没有长度为len的数据，则返回false
bool EventService::ReadBuf(char* buf, size_t len)
{
	return _bufReader.Read(buf, len);
}

std::string EventService::ReadAll()
{
	return _bufReader.ReadAll();
}

//读一行，该行以\r\n结尾,若没有，返回空串
std::string EventService::ReadLineEndOfRN()
{
	return _bufReader.ReadLineEndOfRN();
}

//读一行，该行以\r结尾,若没有，返回空串
std::string EventService::ReadLineEndOfR()
{
	return _bufReader.ReadLineEndOfR();
}

//读一行，该行以\n结尾,若没有，返回空串
std::string EventService::ReadLineEndOfN()
{
	return _bufReader.ReadLineEndOfN();
}

void EventService::RunAt(Time time, std::function<void()>&& timerCb)
{
	_pMyEvMgr->RunAt(time, std::move(timerCb));
}

void EventService::RunAt(Time time, std::function<void()>& timerCb)
{
	_pMyEvMgr->RunAt(time, timerCb);
}

//time时间后执行timerCb函数
void EventService::RunAfter(Time time, std::function<void()>&& timerCb)
{ 
	_pMyEvMgr->RunAfter(time, std::move(timerCb));
}

//time时间后执行timerCb函数
void EventService::RunAfter(Time time, std::function<void()>& timerCb)
{
	_pMyEvMgr->RunAfter(time, timerCb);
}

//每过time时间执行timerCb函数
void EventService::RunEvery(Time time, std::function<void()>&& timerCb)
{
	_pMyEvMgr->RunEvery(time, timerCb);
}

void EventService::RunEvery(Time time, std::function<void()>& timerCb)
{
	_pMyEvMgr->RunEvery(time, timerCb);
}

//每过time时间执行一次timerCb函数,直到isContinue函数返回false
void EventService::RunEveryUntil(Time time, std::function<void()>& timerCb, std::function<bool()>& isContinue)
{
	_pMyEvMgr->RunEveryUntil(time, timerCb, isContinue);
}

void EventService::RunEveryUntil(Time time, std::function<void()>& timerCb, std::function<bool()>&& isContinue)
{
	_pMyEvMgr->RunEveryUntil(time, timerCb, isContinue);
}

void EventService::RunEveryUntil(Time time, std::function<void()>&& timerCb, std::function<bool()>& isContinue)
{
	_pMyEvMgr->RunEveryUntil(time, timerCb, isContinue);
}

void EventService::RunEveryUntil(Time time, std::function<void()>&& timerCb, std::function<bool()>&& isContinue)
{
	_pMyEvMgr->RunEveryUntil(time, timerCb, isContinue);
}

//运行所有已经超时的需要执行的函数
void EventService::RunExpired()
{
	_pMyEvMgr->RunExpired();
}

//将任务放在线程池中以达到异步执行的效果
void EventService::RunInThreadPool(std::function<void()>&& func)
{
    return _pMyEvMgr->RunInThreadPool(std::move(func));
}
