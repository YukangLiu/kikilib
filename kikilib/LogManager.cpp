//@Author Liu Yukang 
#include "LogManager.h"

using namespace kikilib;

//static成员变量需要在类外初始化
LogManager* LogManager::_logMgr = nullptr;
std::mutex LogManager::_logMutex;
bool LogManager::_isInit = false;

LogManager::LogManager() :
	_curLogFileByte(0),
	_curLogFileIdx(0),
	_logFile(nullptr),
	_logLoop(nullptr),
	_stop(0L),
	_lastRead(-1L),
	_lastWrote(-1L),
	_writableSeq(0L)
{ }

bool LogManager::StartLogManager(std::string logPath)
{
	if (!_isInit)
	{
		std::lock_guard<std::mutex> lock(_logMutex);
		if (!_isInit)
		{
			_logPath = logPath;
			_logFile = ::fopen((_logPath + std::to_string(_curLogFileIdx & 1)).c_str(), "w");
			if (!_logFile)
			{
				printf("logfile open failed");
				return false;
			}
			std::string fileMsg = "cur file idx : 0\n";
			fwrite(fileMsg.c_str(), fileMsg.size(), 1, _logFile);
			fflush(_logFile);
			_isInit = true;
			//创建一个线程持续写日志
			_logLoop = new std::thread(&LogManager::WriteDownLog, this);
		}
	}
	return true;
}

void LogManager::EndLogManager()
{
	_stop.store(1L);
	_condition.notify_one();
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
	Record(std::move(logDate));
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
	Record(std::move(LogDataTypeStr[dataType] + logData));
}

void LogManager::Record(unsigned dataType, std::string&& logData)
{
	Record(std::move(LogDataTypeStr[dataType] + logData));
}

void LogManager::Record(std::string&& logDate)
{
	if (_writableSeq.load() - _lastRead.load() >= Parameter::kLogBufferLen - 1)
	{//满了放弃该条日志，因为这种情况下往往前面的日志更有价值
		return;
	}
	const int64_t writableSeq = _writableSeq.fetch_add(1);
	
	//写操作
	_ringBuf[writableSeq & (Parameter::kLogBufferLen - 1)] = std::move(logDate);
	//volatile int a = 0;
	while (writableSeq - 1L != _lastWrote.load())
	{//因为写操作速度相当，所以这里一般不会阻塞,需要注意的，这个_lastWrote里面的值是volatile的，所以这里这样写才没有问题
	}
	_lastWrote.store(writableSeq);

	if (writableSeq == _lastRead.load() + 1)
	{
		std::unique_lock<std::mutex> lock(_logMutex);
		_condition.notify_one();
	}
}

void LogManager::WriteDownLog()
{
	const int64_t kMaxPerLogFileByte = Parameter::maxLogDiskByte / 2;
	while (true)
	{
		if (_stop.load() && _lastRead.load() == _lastWrote.load())
		{
			return;
		}
		else
		{//只要可写，就必须要写完
			//开始打印当前日志内容队列
			while (_lastRead.load() < _lastWrote.load())
			{
				while (_lastRead.load() < _lastWrote.load())
				{
					int64_t curRead = _lastRead.load() + 1;

					if (_curLogFileByte > kMaxPerLogFileByte)
					{//磁盘要爆满了，舍弃旧的日志
						fflush(_logFile);
						::fclose(_logFile);
						++_curLogFileIdx;
						_logFile = ::fopen((_logPath + std::to_string(_curLogFileIdx & 1)).c_str(), "w");
						_curLogFileByte = 0;
						//首先记录当前日志为第几个日志了
						std::string fileMsg = "cur file idx : " + std::to_string(_curLogFileIdx) + '\n';
						fwrite(fileMsg.c_str(), fileMsg.size(), 1, _logFile);
					}
					std::string& buf = _ringBuf[curRead & (Parameter::kLogBufferLen - 1)];
					buf.append("\n");
					fwrite(buf.c_str(), buf.size(), 1, _logFile);
					_curLogFileByte += buf.size();
					buf.clear();
					_lastRead.store(curRead);
				}
				fflush(_logFile);
			}

			{//如果没数据了，则阻塞
				std::unique_lock<std::mutex> lock(_logMutex);
				if (!_stop.load() && _lastRead.load() == _lastWrote.load())
				{
					_condition.wait(lock);
				}
			}
		}
	}
}