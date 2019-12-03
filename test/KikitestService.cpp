#include "KikitestService.h"
#include "LogManager.h"

void KikitestService::HandleReadEvent()
{
	int len = -4;
	std::string str;
	WriteInt32(15);
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