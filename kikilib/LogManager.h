#pragma once

//#include <exception>
#include <queue>
#include <mutex>
#include <atomic>
#include <string>
#include <stdio.h>
#include <thread>
#include <condition_variable>

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
			_curLogFileByte = 0;
			_curLogFileIdx = 0;
		};

		~LogManager() {};

	public:

		//日志管理者类不允许复制
		DISALLOW_COPY_MOVE_AND_ASSIGN(LogManager);

		//初始化LogManager，调用方法为kikilib::LogManager::GetLogMgr()->InitLogManager(path)
		//在程序启动时必须先调用该函数，否则该类的使用是异常的
		bool StartLogManager(std::string logPath);

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

		//和条件变量配合使用的锁
		std::mutex _conditionMutex;

		//用于唤醒写日志线程
		std::condition_variable _condition;

		//是否已经初始化
		static bool _isInit;

		//当前正在写的日志文件的字节大小
		int64_t _curLogFileByte;

		//当前正在写的日志文件是已写的第几个日志文件，实际永远只会有两个文件，当磁盘满了舍弃旧的一个重新写
		int64_t _curLogFileIdx;

		//日志文件路径
		std::string _logPath;

		//日志文件,测试证明比ofstream更快，故使用fwrite
		FILE* _logFile;

		//写日志的线程
		std::thread* _logLoop;

		//两个队列，一条用于线程取数据写日志，另一条用于添加throw出来的异常
		//当线程写日志清空队列后，修改_recordableQue的值，然后对另外一条队列进行写日志
		std::queue<std::string> _logQue[2];

		//当前可用于存入需写数据的队列
		std::atomic_int _recordableQue;

		bool _stop;
	};
}
