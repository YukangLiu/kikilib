//@Author Liu Yukang 
#include "KikitestService.h"
#include "LogManager.h"
#include "Time.h"

#include <stdio.h>

void KikitestService::handleReadEvent()
{
	int len = -4;
	std::string str;
	sendInt32(15);
	kikilib::Time start(0);
	kikilib::Time end(0);
	while (readInt32(len))
	{
		kikilib::TimerTaskId t;
		switch (len)
		{
		case 0:
			str = readLineEndOfN();
			RecordLog(str);
			break;

		case -1:
			str = readLineEndOfR();
			RecordLog(str);
			break;

		case -2:
			str = readAll();
			runInThreadPool(
				[this]
				{
					this->modifyVal();
				}
			);
			runEvery(150,
				[this]
				{
					RecordLog(std::to_string(this->getVal()));
				}
					,t
				);
			break;

		case -3:
			str = readAll();
			runAfter(
				1000000,
				[this]
				{
					this->modifyVal();
				}
				);
			runEveryUntil(20000,
				[this]
				{
					RecordLog(std::to_string(this->getVal()));
				},
				[this]()
					->bool
				{
					return this->getVal() != 100;
				}
					, t
				);
			break;

		case -4:
			str = readAll();
			//我的机器上用双缓存队列50000000条数据是11.26sec
			//我的机器上用环形队列50000000条数据是5.26sec，快了一倍
			start = kikilib::Time::now();
			for (int i = 0; i < 50000000; ++i)
			{
				RecordLog(DEBUG_DATA_INFORMATION,std::to_string(i));
			}
			end = kikilib::Time::now();
			printf("%dms\n", (int)(end.getTimeVal() - start.getTimeVal()) / 1000);
			break;

		default:
			str = readBuf(len);
			RecordLog(str);
			break;
		}
	}
	
};

void KikitestService::handleErrEvent()
{
	forceClose();
};