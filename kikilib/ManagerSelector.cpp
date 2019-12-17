#include "ManagerSelector.h"
#include "EventManager.h"

using namespace kikilib;

void ManagerSelector::setStrategy(int strategy)
{
	_strategy = strategy;
}

EventManager* ManagerSelector::next()
{
	int n = static_cast<int>(_evMgrs.size());
	if (!n)
	{
		return nullptr;
	}
	int minEvMgrIdx = 0;
	size_t minEvCnt = _evMgrs.front()->eventServeCnt();
	switch (_strategy)
	{
	case MIN_EVENT_FIRST:
		for (int i = 1; i < n ; ++i)
		{
			if (_evMgrs[i]->eventServeCnt() < minEvCnt)
			{
				minEvCnt = _evMgrs[i]->eventServeCnt();
				minEvMgrIdx = i;
			}
		}
		_curMgr = minEvMgrIdx;
		break;

	case ROUND_ROBIN:
	default:
		++_curMgr;
		if (_curMgr >= n)
		{
			_curMgr = 0;
		}
		break;
	}
	return _evMgrs[_curMgr];
};