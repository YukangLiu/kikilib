#include "EventService.h"
#include "EventManager.h"
#include "LogManager.h"

#include <poll.h>
#include <sys/epoll.h>

using namespace kikilib;

EventService::EventService(Socket sock, EventManager* evMgr, int interestEvent)
	: _sock(sock), _pMyEvMgr(evMgr), _interestEvent(interestEvent), _isConnected(true), \
	_bufReader(sock), _bufWritter(sock, this), _eventPriority(NORMAL_EVENT)
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

void EventService::SetInteresEv(int newInterestEv)
{
	_interestEvent = newInterestEv;
	_pMyEvMgr->Motify(this);
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
		ssize_t n = _bufReader.ReadFillBuf();
		if (n > 0)
		{
			HandleReadEvent();
		}
		else if (n == 0)
		{
			Close();
		}
		else
		{
			RecordLog(ERROR_DATA_INFORMATION, "read fd error!");
			HandleErrEvent();
		}
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

//time时间后执行timerCb函数
void EventService::RunAfter(Time time, std::function<void()> timerCb)
{ 
	_pMyEvMgr->RunAfter(time, timerCb);
}

//每过time时间执行timerCb函数
void EventService::RunEvery(Time time, std::function<void()> timerCb) 
{ 
	_pMyEvMgr->RunEvery(time, timerCb); 
}

//将任务放在线程池中以达到异步执行的效果
template<class F, class... Args>
auto EventService::RunInThreadPool(F&& f, Args&&... args)
->std::future<typename std::result_of<F(Args...)>::type>
{
    return _pMyEvMgr->RunInThreadPool(f,args...);
}
