//@Author Liu Yukang 
#pragma once
#include "utils.h"

#include <vector>

struct epoll_event;

namespace kikilib
{
	class EventService;

	//事件监视器
	//职责：
	//1、监视发现当前被触发的事件
	//2、修改被监视的事件
	//3、该类不保证线程安全，线程安全由EventManager保证
	//注：这里的epoll用的LT模式，原因如下：
	//	  read事件到来时，若server的业务并不会每次readall并进行及时处理，那么，如果遭遇client
	//疯狂发送巨大包体，ET模式必须每次将内容读进内存，而server不及时处理就会导致内容堆积，内存
	//爆满，使用LT不会出现这个问题，socket接收缓冲区是满的对方会发送失败。
	class EventEpoller
	{
	public:
		EventEpoller();
		~EventEpoller();

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventEpoller);

		//要使用EventEpoller必须调用该函数初始化，失败则返回false
		bool init();

		//修改EventEpoller中的事件
		void modifyEv(EventService* evServ);

		//向EventEpoller中添加事件
		void addEv(EventService* evServ);

		//从EventEpoller中移除事件
		void removeEv(EventService* evServ);

		//获取被激活的事件服务
		void getActEvServ(int timeOutMs, std::vector<EventService*>& activeEvServs);

	private:

		bool isEpollFdUsefulAndMark();

		int _epollFd;
		std::vector<struct epoll_event> _activeEpollEvents;

	};

}
