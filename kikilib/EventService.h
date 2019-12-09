//@Author Liu Yukang 
#pragma once

#include "Socket.h"
#include "SocketReader.h"
#include "SocketWritter.h"
#include "Time.h"
#include "utils.h"

#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <functional>
#include <future>

namespace kikilib
{
	class EventManager;

	//事件服务员类
	//职责：
	//1、专属于一个socket，专注于服务该socket上发生的事件
	//2、提供自身socket的操作API
	//3、提供自身事件相关的操作API
	//4、提供定时器相关的操作API
	//5、提供线程池工具的操作API
	//6、提供socket缓冲区的读写操作API
	//使用方法：
	//1、用户继承该类，实现其中的HandleConnectionEvent(),
	//   HandleReadEvent(),HandleErrEvent,HandleCloseEvent()
	//   函数即可，可自定义自己的私有成员，代替了大多数网络库
	//   中的context上下文指针，生命器管理也更容易
	//2、使用EventMaster时，将该类型放在EventMaster的模板中
	class EventService
	{
	public:
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////用户API//////////////////////////////////////////////////////

		//////////////////////////////自身socket的操作API//////////////////////////////////

		int GetFd() { return _sock.fd(); };

		std::string GetPeerIP() { return _sock.GetIp(); };

		int GetPeerPort() { return _sock.GetPort(); };

		//获取套接字的选项的字符串
		std::string GetSocketOptString() { return _sock.GetSocketOptString(); };

		//关闭当前事件
		void Close();
		void ShutDownWrite();

		///////////////////////////////自身事件的操作API///////////////////////////////////

		//获取当前事件服务所关注的事件
		int GetInteresEv() { return _interestEvent; };
		
		//修改当前事件服务所关注的事件
		void SetInteresEv(int newInterestEv);

		//获取当前事件的优先级
		unsigned GetEventPriority() { return _eventPriority; };

		//修改当前事件的优先级
		void SetEventPriority(unsigned priority) { _eventPriority = priority; };

		/////////////////////////////////事件管理的操作API///////////////////////////////////

		//向事件管理器中插入一个事件,这是线程安全的
		void Insert(EventService* ev);

		//向事件管理器中移除一个事件,这是线程安全的
		void Remove(EventService* ev);

		//向事件管理器中修改一个事件服务所关注的事件类型,这是线程安全的
		void Motify(EventService* ev);

		//获得EventManager区域唯一的上下文内容
		void* GetEvMgrCtx();

		///////////////////////////////定时器相关的操作API///////////////////////////////////

		//需要注意，如果timerCb里面会执行RunExpired()函数的话会发生死锁

		//在time时刻执行timerCb函数
		void RunAt(Time time, std::function<void()>&& timerCb);
		void RunAt(Time time, std::function<void()>& timerCb);

		//time时间后执行timerCb函数
		void RunAfter(Time time, std::function<void()>&& timerCb);
		void RunAfter(Time time, std::function<void()>& timerCb);

		//虽然调用eventmanager的下面这几个函数是必须拷贝的，但是这里弄成这样可以减少一次拷贝
		//每过time时间执行timerCb函数
		void RunEvery(Time time, std::function<void()>&& timerCb);
		void RunEvery(Time time, std::function<void()>& timerCb);

		//每过time时间执行一次timerCb函数,直到isContinue函数返回false
		void RunEveryUntil(Time time, std::function<void()>& timerCb, std::function<bool()>& isContinue);
		void RunEveryUntil(Time time, std::function<void()>& timerCb, std::function<bool()>&& isContinue);
		void RunEveryUntil(Time time, std::function<void()>&& timerCb, std::function<bool()>& isContinue);
		void RunEveryUntil(Time time, std::function<void()>&& timerCb, std::function<bool()>&& isContinue);

		//运行所有已经超时的需要执行的函数
		void RunExpired();

		///////////////////////////////线程池相关的操作API///////////////////////////////////

		//将任务函数放在线程池中以达到异步执行的效果，如：
		//1、向数据库写数据，直接将写数据库的函数放入其中
		//2、从数据库读数据，将读取数据库的函数放入其中，
		//   然后设置定时器事件，过time时间后检查是否读完
        void RunInThreadPool(std::function<void()>&& func);

		////////////////////////////socket缓冲区相关的操作API/////////////////////////////////

		//向socket写一个int数字num
		void WriteInt32(int num);
		
		//向socket写内容content
		void WriteBuf(std::string& content);
		void WriteBuf(std::string&& content);

		//读取一个int，若缓存中没有，则返回false
		bool ReadInt32(int& res);

		//读取长度为len的字符,若没有长度为len的数据，则返回空串
		std::string ReadBuf(size_t len);

		//读取长度为len的数据，若没有长度为len的数据，则返回false
		bool ReadBuf(char* buf, size_t len);

		//读一行，该行以\r\n结尾,若没有，返回空串
		std::string ReadLineEndOfRN();

		//读一行，该行以\r结尾,若没有，返回空串
		std::string ReadLineEndOfR();

		//读一行，该行以\n结尾,若没有，返回空串
		std::string ReadLineEndOfN();

		//读取缓冲区中的所有字符
		std::string ReadAll();

		////////////////////////////////////////////////用户API/////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////
		 

		//////////////////////////运行接口,需要用户子类自己实现///////////////////////////
		//新的连接到来时执行的函数，实际会在insert进EventManager时调用
		virtual void HandleConnectionEvent() {};

		//可读事件到来时执行的函数
		virtual void HandleReadEvent() {};

		//错误事件到来时执行的函数
		virtual void HandleErrEvent() {};

		//连接关闭时执行的回调函数
		virtual void HandleCloseEvent() {};

		///////////////////////////供EventMaster和EventManager调用////////////////////////////////

		//设置事件状态，如可读，可写，错误
		void SetEventState(int state) { _eventState = state; };

		//EventManager会调用该函数根据事件类型处理事件
		//用户可以重写，但不建议
		virtual void HandleEvent();

		//连接是否已经关闭
		bool IsConnected() { return _isConnected; }

	public:
		EventService(Socket& sock, EventManager* evMgr, int interestEvent = EPOLLIN | EPOLLPRI | EPOLLRDHUP);
		EventService(Socket&& sock, EventManager* evMgr, int interestEvent = EPOLLIN | EPOLLPRI | EPOLLRDHUP);

		virtual ~EventService() {};

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventService);

	private:
		//写事件
		void HandleWriteEvent();

		//关注的事件，即该EventBody在epoll中会被什么事件触发
		int _interestEvent;

		//事件状态：可写事件，可读事件等
		int _eventState;

		//事件优先级，默认为NORMAL_EVENT
		unsigned _eventPriority;

		//是否已经连接
		bool _isConnected;

		//该事件服务的socket
		Socket _sock;

		//该事件所属的事件管理器
		EventManager* _pMyEvMgr;

		SocketWtitter _bufWritter;

		SocketReader _bufReader;
	};

}