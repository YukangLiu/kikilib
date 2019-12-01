#pragma once

//#include <exception>
#include <queue>
#include <mutex>
#include <atomic>
#include <string>
#include <stdio.h>
#include <thread>

#include "utils.h"
//#include <stdarg.h>

//日志信息类型
enum LogDataType
{
	DEBUG_DATA_INFORMATION = 0, //记录普通信息
	WARNING_DATA_INFORMATION,  //记录警告信息
	ERROR_DATA_INFORMATION, //记录运行错误
};

////////////////////////////////////全局使用的API/////////////////////////////////
//打开日志管理器
#define StartLogMgr(logPath) (kikilib::LogManager::GetLogMgr()->StartLogManager((logPath)))
//关闭日志管理器
#define EndLogMgr() (kikilib::LogManager::GetLogMgr()->EndLogManager())
//记录日志，使用该宏即可
#define RecordLog(...) (kikilib::LogManager::GetLogMgr()->Record(__VA_ARGS__))

namespace kikilib
{
	//对应LogDataType的宏
	static std::string LogDataTypeStr[3] =
	{
		"remark : ", //记录普通信息
		"warning : ",  //记录警告信息
		"error : ", //记录运行错误
	};

	//日志管理者类，此类为单例，使用双缓冲队列和独立的写日志线程实现
	//职责：
	//缓冲区有日志信息需要写的时候，将其写到日志文件中，否则睡眠一段时间
	//使用方法：
	//1、程序启动时候调用StartLogMgr()
	//2、程序结束前调用EndLogMgr()
	//3、使用RecordLog()函数记录日志信息
	class LogManager
	{
	protected:
		LogManager() {
			_stop = false;
			_isInit = false;
			_recordableQue.store(0);
			_isWritable.store(false);
		};

		~LogManager() {};

	public:

		//日志管理者类不允许复制
		DISALLOW_COPY_MOVE_AND_ASSIGN(LogManager);

		//初始化LogManager，调用方法为kikilib::LogManager::GetLogMgr()->InitLogManager(path)
		//在程序启动时必须先调用该函数，否则该类的使用是异常的
		bool StartLogManager(std::string logPath);
		bool StartLogManager(const char* logPath);

		//结束日志管理器，关闭进程前必须执行
		void EndLogManager();

		//获取日志管理对象实例
		//若没有初始化则返回nullptr
		static LogManager* GetLogMgr();

		//记录日志
		void Record(const char* logData);
		void Record(std::string& logData);
		void Record(std::string&& logData);
		void Record(unsigned dataType, const char* logData);
		void Record(unsigned dataType, std::string& logData);
		void Record(unsigned dataType, std::string&& logData);

	private:
		//向磁盘写入日志
		void WriteDownLog();

	private:
		//日志管理器实例
		static LogManager* _logMgr;

		//用于保护的锁，为了服务器执行效率，原则上不允许长久占有此锁
		static std::mutex _logMutex;

		//是否已经初始化
		static bool _isInit;

		//日志文件,测试证明比ofstream更快，故使用fwrite
		FILE* _logFile;

		//写日志的线程
		std::thread* _logLoop;

		//两个队列，一条用于线程取数据写日志，另一条用于添加throw出来的异常
		//当线程写日志清空队列后，修改_recordableQue的值，然后对另外一条队列进行写日志
		std::queue<std::string> _logQue[2];

		//当前可用于存入需写数据的队列
		std::atomic_int _recordableQue;

		//当前记录线程是否可向日志文件中写入日志，当队列数大于10时，将该对象设为true
		std::atomic_bool _isWritable;

		bool _stop;
	};
}
