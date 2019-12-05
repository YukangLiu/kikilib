//@Author Liu Yukang 
#include "KikitestService.h"
#include "LogManager.h"
#include "Time.h"

#include <stdio.h>

void KikitestService::HandleReadEvent()
{
	int len = -4;
	std::string str;
	WriteInt32(15);
	kikilib::Time start(0);
	kikilib::Time end(0);
	while (ReadInt32(len))
	{
		switch (len)
		{
		case 0:
			str = ReadLineEndOfN();
			RecordLog(str);
			break;

		case -1:
			str = ReadLineEndOfR();
			RecordLog(str);
			break;

		case -2:
			str = ReadAll();
			RunInThreadPool(
				[this]
				{
					this->MotifyVal();
				}
			);
			RunEvery(150,
				[this]
				{
					RecordLog(std::to_string(this->GetVal()));
				}
				);
			break;

		case -3:
			//我的机器上用双缓存队列50000000条数据是11.26sec
			start = kikilib::Time::now();
			for (int i = 0; i < 50000000; ++i)
			{
				RecordLog(DEBUG_DATA_INFORMATION,std::to_string(i));
			}
			end = kikilib::Time::now();
			printf("%dms\n", (int)(end.GetTimeVal() - start.GetTimeVal()));
			break;

		default:
			str = ReadBuf(len);
			RecordLog(str);
			break;
		}
	}
	
};

void KikitestService::HandleErrEvent()
{
	Close();
};