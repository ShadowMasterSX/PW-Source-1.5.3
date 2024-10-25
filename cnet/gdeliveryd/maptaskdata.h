#ifndef __GNET_MAPTASKDATA_H
#define __GNET_MAPTASKDATA_H

#include <map>

#include "thread.h"

namespace GNET
{

class TaskData
{
	typedef std::map<int/*task id*/,Octets/*task data*/> Map;
	Map map;
	Thread::Mutex locker;
	static TaskData	instance;
public:
	TaskData() : locker("TaskData::locker") { }
	~TaskData() { }
	static TaskData & GetInstance() { return instance; }
	size_t Size() { Thread::Mutex::Scoped l(locker);	return map.size();	}

	bool GetTaskData(int taskid, Octets &taskdata);

	void GameSetTaskData(int taskid, const Octets &td);
	void SetTaskData(int taskid, const Octets &td );
	void GameGetTaskData(int taskid, unsigned int sid,int playerid, const Octets &env);

};

};

#endif

