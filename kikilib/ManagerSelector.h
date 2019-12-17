//@Author Liu Yukang 
#pragma once

namespace kikilib
{
	//事件管理器选择器，决定下一个事件应该放入哪个事件管理器中
	class ManagerSelector
	{
	public:
		ManagerSelector(int mgrCnt = 0) : _mgrCnt(mgrCnt), _curMgr(-1) {}

		void setManagerCnt(int mgrCnt) { _mgrCnt = mgrCnt; };

		void setStrategy(int);

		int next() 
		{
			++_curMgr;
			if (_curMgr >= _mgrCnt)
			{
				_curMgr = 0;
			}
			return _curMgr;
		};

	private:
		int _mgrCnt;
		int _curMgr;

	};

}