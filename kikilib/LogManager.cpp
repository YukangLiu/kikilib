#include "LogManager.h"
#include "Parameter.h"

#define MIN_WRITABLE_LOGDATE_NUM 1 //当队列中数据小于这个数时，写日志的线程等待这么多WRITING_LOG_WAIT_TIME毫秒
#define WRITING_LOG_WAIT_TIME 100 //当队列中数据小于MIN_WRITABLE_LOGDATE_NUM时，写日志的线程等待这么多ms
#define BYTE_PER_LOGQUE_STRING 60 //日志队列每个string元素的字节大小估算值

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
			_logPath = logPath;
			_logFile = ::fopen((_logPath + std::to_string(_logFileIdx)).c_str(), "w");
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
	const int64_t kMaxPerLogFileByte = Parameter::maxLogDiskByte / 2;
	while (true)
	{
		if (_isWritable.load())
		{//只要可写，就必须要写完
			//日志内容先记录到另一队列中
			_isWritable.store(false);
			int curWritingQue = _recordableQue.load();
			_recordableQue.store(!curWritingQue);

			//开始打印当前日志内容队列
			if ((Parameter::maxLogQueueByte / 2) < (_logQue[curWritingQue].size() * BYTE_PER_LOGQUE_STRING) && _logQue[!curWritingQue].size())
			{
				//队列每个string按BYTE_PER_LOGQUE_STRING字节估算，若当前队列日志缓存占用内存太大
				//则意味着RecordLog的速度很快，是某个地方发生了剧烈的错误，这种时候，往往只是前面一些
				//日志是关键的，后面的日志则都无关紧要，故立即舍弃掉当前队列，以防止内存爆满，同时追上
				//RecordLog的速度
				while (!_logQue[curWritingQue].empty())
				{
					_logQue[curWritingQue].pop();
				}
			}
			else
			{
				while (!_logQue[curWritingQue].empty())
				{
					if (_curLogFileByte > kMaxPerLogFileByte)
					{//磁盘要爆满了，舍弃旧的日志
						::fclose(_logFile);
						_logFileIdx = !_logFileIdx;
						_logFile = ::fopen((_logPath + std::to_string(_logFileIdx)).c_str(), "w");
						_curLogFileByte = 0;
					}
					std::string& buf = _logQue[curWritingQue].front();
					buf.append("\n");
					fwrite(buf.c_str(), buf.size(), 1, _logFile);
					_logQue[curWritingQue].pop();
					_curLogFileByte += BYTE_PER_LOGQUE_STRING;
				}
				fflush(_logFile);
			}
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