#include "LogManager.h"

#define MIN_WRITABLE_LOGDATE_NUM 1 //当队列中数据小于这个数时，写日志的线程等待这么多WRITING_LOG_WAIT_TIME毫秒
#define WRITING_LOG_WAIT_TIME 100 //当队列中数据小于MIN_WRITABLE_LOGDATE_NUM时，写日志的线程等待这么多ms

using namespace kikilib;

//static成员变量需要在类外初始化
LogManager* LogManager::_logMgr = nullptr;
std::mutex LogManager::_logMutex;
bool LogManager::_isInit = false;

bool LogManager::StartLogManager(std::string logPath)
{
	if (!_isInit)
	{
		std::lock_guard<std::mutex> lock(_logMutex);
		if (!_isInit)
		{
			_logFile = ::fopen(logPath.c_str(), "w");
			if (!_logFile)
			{
				return false;
			}
			_isInit = true;
			//创建一个线程持续写日志
			_logLoop = new std::thread(&LogManager::WriteDownLog, this);
		}
	}
	return true;
}

bool LogManager::StartLogManager(const char* logPath)
{
	if (!_isInit)
	{
		std::lock_guard<std::mutex> lock(_logMutex);
		if (!_isInit)
		{
			_logFile = fopen(logPath, "w");
			if (!_logFile)
			{
				return false;
			}
			_isInit = true;
			//创建一个线程持续写日志
			_logLoop = new std::thread(&LogManager::WriteDownLog, this);
		}
	}
	return true;
}

void LogManager::EndLogManager()
{
	_stop = true;
	_logLoop->join();
	::fclose(_logFile);
	_isInit = false;
	delete _logLoop;
}

LogManager* LogManager::GetLogMgr()
{
	if (!_logMgr)
	{
		std::lock_guard<std::mutex> lock(_logMutex);
		if (!_logMgr)
		{
			_logMgr = new LogManager();
		}
	}
	//else if (!_isInit)
	//{
	//	return nullptr;
	//}
	return _logMgr;
}

void LogManager::Record(std::string& logDate)
{
	std::lock_guard<std::mutex> lock(_logMutex);//线程安全
	int curQue = _recordableQue.load();
	_logQue[curQue].emplace(logDate);
	if (_logQue[curQue].size() == MIN_WRITABLE_LOGDATE_NUM)
	{
		_isWritable.store(true);
	}
}

void LogManager::Record(std::string&& logDate)
{
	std::lock_guard<std::mutex> lock(_logMutex);//线程安全
	int curQue = _recordableQue.load();
	_logQue[curQue].emplace(std::move(logDate));
	if (_logQue[curQue].size() == MIN_WRITABLE_LOGDATE_NUM)
	{
		_isWritable.store(true);
	}
}

void LogManager::Record(const char* logData)
{
	Record(std::move(std::string(logData)));
}

void LogManager::Record(unsigned dataType, const char* logData)
{
	Record(std::move(std::string(LogDataTypeStr[dataType] + logData)));
}

void LogManager::Record(unsigned dataType, std::string& logData)
{
	Record(std::move(LogDataTypeStr[dataType] +  logData));
}

void LogManager::Record(unsigned dataType, std::string&& logData)
{
	Record(std::move(LogDataTypeStr[dataType] +  logData));
}

void LogManager::WriteDownLog()
{
	while (true)
	{
		if (_isWritable.load())
		{//只要可写，就必须要写完
			//日志内容先记录到另一队列中
			_isWritable.store(false);
			int curWritingQue = _recordableQue.load();
			_recordableQue.store(!curWritingQue);

			//开始打印当前日志内容队列
			while (!_logQue[curWritingQue].empty())
			{
				std::string& buf = _logQue[curWritingQue].front();
				buf.append("\n");
				fwrite(buf.c_str(), buf.size(), 1, _logFile);
				_logQue[curWritingQue].pop();
			}
			fflush(_logFile);
		}
		else if (_stop)
		{
			return;
		}
		else
		{//数据不够写，先等一段时间
			std::this_thread::sleep_for(std::chrono::milliseconds(WRITING_LOG_WAIT_TIME));
		}
	}
}