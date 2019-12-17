//@Author Liu Yukang 
#pragma once

//#include <exception>
#include <queue>
#include <mutex>
#include <atomic>
#include <string>
#include <stdio.h>
#include <thread>
#include <condition_variable>
#include <array>

#include "Time.h"
#include "Parameter.h"
#include "Sequence.h"
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
#define StartLogMgr(logPath) (kikilib::LogManager::getLogMgr()->startLogManager((logPath)))
//关闭日志管理器
#define EndLogMgr() (kikilib::LogManager::getLogMgr()->endLogManager())
//记录日志，使用该宏即可
//注意，为了效率，所有的日志内容都是转移的，即调用完该函数，原始string内容会没有了，用户若需要备份需要自己拷贝一份
#define RecordLog(...) (kikilib::LogManager::getLogMgr()->recordInLog(__VA_ARGS__))

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
		LogManager();

		~LogManager() {};

	public:

		DISALLOW_COPY_MOVE_AND_ASSIGN(LogManager);

		//初始化LogManager，调用方法为kikilib::LogManager::GetLogMgr()->InitLogManager(path)
		//在程序启动时必须先调用该函数，否则该类的使用是异常的
		bool startLogManager(std::string logPath);

		//结束日志管理器，关闭进程前必须执行
		void endLogManager();

		//获取日志管理对象实例
		//若没有初始化则返回nullptr
		static LogManager* getLogMgr();

		//记录日志
		void recordInLog(const char* logData);
		void recordInLog(std::string& logData);
		void recordInLog(std::string&& logData);
		void recordInLog(unsigned dataType, const char* logData);
		void recordInLog(unsigned dataType, std::string& logData);
		void recordInLog(unsigned dataType, std::string&& logData);

	private:
		//向磁盘写入日志
		void writeDownLog();

		//更新日志系统的时间
		void updateLogTime();

	private:
		//日志管理器实例
		static LogManager* _logMgr;

		//用于保护的锁，为了服务器执行效率，原则上不允许长久占有此锁
		static std::mutex _logMutex;

		//是否已经初始化
		static bool _isInit;

		//用于唤醒写日志线程
		std::condition_variable _condition;

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

		Sequence _stop;

		//最后一个已读内容位置
		Sequence _lastRead;

		//最后一个已写内容位置
		Sequence _lastWrote;

		//当前可写的槽位序号
		AtomicSequence _writableSeq;

		std::mutex _timeMutex;

		std::string _logTimeStr;

		time_t _logTimeSec;

		//时区偏置,单位为秒
		long _timeZone;

		std::array<std::string, Parameter::kLogBufferLen> _ringBuf;
	};
}
