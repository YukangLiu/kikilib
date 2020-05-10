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
	typedef Time TimerTaskId;

	//�¼�����Ա��
	//ְ��
	//1��ר����һ��socket��רע�ڷ����socket�Ϸ������¼�
	//2���ṩ����socket�Ĳ���API
	//3���ṩ�����¼���صĲ���API
	//4���ṩ��ʱ����صĲ���API
	//5���ṩ�̳߳ع��ߵĲ���API
	//6���ṩsocket�������Ķ�д����API
	//ʹ�÷�����
	//1���û��̳и��࣬ʵ�����е�HandleConnectionEvent(),
	//   HandleReadEvent(),HandleErrEvent,HandleCloseEvent()
	//   �������ɣ����Զ����Լ���˽�г�Ա�������˴���������
	//   �е�context������ָ�룬����������Ҳ������
	//2��ʹ��EventMasterʱ���������ͷ���EventMaster��ģ����
	class EventService
	{
	public:
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////�û�API//////////////////////////////////////////////////////

		//////////////////////////////����socket�Ĳ���API//////////////////////////////////

		int fd() { return _sock.fd(); };

		std::string peerIP() { return _sock.ip(); };

		int peerPort() { return _sock.port(); };

		//��ȡ�׽��ֵ�ѡ����ַ���
		std::string getSocketOptString() { return _sock.getSocketOptString(); };

		//�رյ�ǰ�¼�
		void forceClose();
		void shutDownWrite();

		///////////////////////////////�����¼��Ĳ���API///////////////////////////////////

		//��ȡ��ǰ�¼���������ע���¼�
		int getInteresEv() { return _interestEvent; };
		
		//�޸ĵ�ǰ�¼���������ע���¼�
		void setInteresEv(int newInterestEv);

		//��ȡ��ǰ�¼������ȼ�
		unsigned getEventPriority() { return _eventPriority; };

		//�޸ĵ�ǰ�¼������ȼ�
		void setEventPriority(unsigned priority) { _eventPriority = priority; };

		/////////////////////////////////�¼������Ĳ���API///////////////////////////////////

		//���¼��������в���һ���¼�,�����̰߳�ȫ��
		void insertEvInThisEvMgr(EventService* ev);

		//���¼����������Ƴ�һ���¼�,�����̰߳�ȫ��
		void removeEvInThisEvMgr(EventService* ev);

		//���¼����������޸�һ���¼���������ע���¼�����,�����̰߳�ȫ��
		void modifyEvInThisEvMgr(EventService* ev);

		//���EventManager����Ψһ������������
		void* getThisEvMgrCtx();

		///////////////////////////////��ʱ����صĲ���API///////////////////////////////////

		//��Ҫע�⣬���timerCb�����ִ��RunExpired()�����Ļ��ᷢ������

		//��timeʱ��ִ��timerCb����
		TimerTaskId runAt(Time time, std::function<void()>&& timerCb);
		TimerTaskId runAt(Time time, std::function<void()>& timerCb);

		//timeʱ���ִ��timerCb����
		TimerTaskId runAfter(Time time, std::function<void()>&& timerCb);
		TimerTaskId runAfter(Time time, std::function<void()>& timerCb);

		//��Ȼ����eventmanager�������⼸�������Ǳ��뿽���ģ���������Ū���������Լ���һ�ο���
		//ÿ��timeʱ��ִ��timerCb����
		void runEvery(Time time, std::function<void()>&& timerCb, TimerTaskId& retId);
		void runEvery(Time time, std::function<void()>& timerCb, TimerTaskId& retId);

		//ÿ��timeʱ��ִ��һ��timerCb����,ֱ��isContinue��������false
		void runEveryUntil(Time time, std::function<void()>& timerCb, std::function<bool()>& isContinue, TimerTaskId& retId);
		void runEveryUntil(Time time, std::function<void()>& timerCb, std::function<bool()>&& isContinue, TimerTaskId& retId);
		void runEveryUntil(Time time, std::function<void()>&& timerCb, std::function<bool()>& isContinue, TimerTaskId& retId);
		void runEveryUntil(Time time, std::function<void()>&& timerCb, std::function<bool()>&& isContinue, TimerTaskId& retId);

		//���������Ѿ���ʱ����Ҫִ�еĺ���
		void runExpired();

		///////////////////////////////�̳߳���صĲ���API///////////////////////////////////

		//�������������̳߳����Դﵽ�첽ִ�е�Ч�����磺
		//1�������ݿ�д���ݣ�ֱ�ӽ�д���ݿ�ĺ�����������
		//2�������ݿ�����ݣ�����ȡ���ݿ�ĺ����������У�
		//   Ȼ�����ö�ʱ���¼�����timeʱ������Ƿ����
        void runInThreadPool(std::function<void()>&& func);

		////////////////////////////socket��������صĲ���API/////////////////////////////////

		//��socketдһ��int����num
		bool sendInt32(int num);
		
		//��socketд����content
		bool sendContent(std::string& content);
		bool sendContent(std::string&& content);

		//��ȡһ��int����������û�У��򷵻�false
		bool readInt32(int& res);

		//��ȡ����Ϊlen���ַ�,��û�г���Ϊlen�����ݣ��򷵻ؿմ�
		std::string readBuf(size_t len);

		//��ȡ����Ϊlen�����ݣ���û�г���Ϊlen�����ݣ��򷵻�false
		bool readBuf(char* buf, size_t len);

		//��һ�У�������\r\n��β,��û�У����ؿմ�
		std::string readLineEndOfRN();

		//��һ�У�������\r��β,��û�У����ؿմ�
		std::string readLineEndOfR();

		//��һ�У�������\n��β,��û�У����ؿմ�
		std::string readLineEndOfN();

		//��ȡ�������е������ַ�
		std::string readAll();

		////////////////////////////////////////////////�û�API/////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////
		 

		//////////////////////////���нӿ�,��Ҫ�û������Լ�ʵ��///////////////////////////
		//�µ����ӵ���ʱִ�еĺ�����ʵ�ʻ���insert��EventManagerʱ����
		virtual void handleConnectionEvent() {};

		//�ɶ��¼�����ʱִ�еĺ���
		virtual void handleReadEvent() {};

		//�����¼�����ʱִ�еĺ���
		virtual void handleErrEvent() {};

		//���ӹر�ʱִ�еĻص�����
		virtual void handleCloseEvent() {};

		///////////////////////////��EventMaster��EventManager����////////////////////////////////

		//�����¼�״̬����ɶ�����д������
		void setEventState(int state) { _eventState = state; };

		//EventManager����øú��������¼����ʹ����¼�
		//�û�������д����������
		virtual void handleEvent();

		//�����Ƿ��Ѿ��ر�
		bool isConnected() { return _isConnected; }

	public:
		EventService(Socket& sock, EventManager* evMgr, int interestEvent = EPOLLIN | EPOLLPRI | EPOLLRDHUP | EPOLLHUP);
		EventService(Socket&& sock, EventManager* evMgr, int interestEvent = EPOLLIN | EPOLLPRI | EPOLLRDHUP | EPOLLHUP);

		virtual ~EventService() {};

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventService);

	private:
		//д�¼�
		void handleWriteEvent();

		//��ע���¼�������EventBody��epoll�лᱻʲô�¼�����
		int _interestEvent;

		//�¼�״̬����д�¼����ɶ��¼���
		int _eventState;

		//�¼����ȼ���Ĭ��ΪNORMAL_EVENT
		unsigned _eventPriority;

		//�Ƿ��Ѿ�����
		bool _isConnected;

		//���¼������socket
		Socket _sock;

		//���¼��������¼�������
		EventManager* _pMyEvMgr;

		SocketWtitter _bufWritter;

		SocketReader _bufReader;
	};

}