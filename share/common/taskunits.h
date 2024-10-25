#ifndef __GNET_TASKUNITS_H
#define __GNET_TASKUNITS_H

#include "thread.h"
#include "objectchangesupport.h"
#include "reference.h"
#include <set>
#include <algorithm>
#include <functional>

namespace GNET
{
namespace Thread
{

class TrueTask : public Thread::StatefulRunnable
{
public:
	TrueTask() { }
	int GetState() const { return SUCCESS; }
	void Run() { FireObjectChange(); }
};

class FalseTask : public Thread::StatefulRunnable
{
public:
	FalseTask() { }
	int GetState() const { return FAIL; }
	void Run() { FireObjectChange(); }
};

class IfAndTask : public Thread::StatefulRunnable
{
	int state;
	TaskGraph::NodeSet	prev;
public:
	IfAndTask() : state(INIT) {	}
	void AddCond(Node * n) { prev.insert(n); }

	int GetState() const { return state; }
	void Run()
	{
		bool b = true;
		for( NodeSet::iterator it = prev.begin(); it != prev.end(); ++ it )
		{
			if( (*it).GetState() != SUCCEED )
				b = false;
		}
		if( b )
			state = SUCCEED;
		else
			state = STOPPED;
		FireObjectChange();
	}
};

class IfOrTask : public Thread::StatefulRunnable
{
	int state;
	TaskGraph::NodeSet	prev;
public:
	IfOrTask() : state(INIT) {	}
	void AddCond(TaskGraph::Node * n) { prev.insert(n); }

	int GetState() const { return state; }
	void Run()
	{
		bool b = false;
		for( NodeSet::iterator it = prev.begin(); it != prev.end(); ++ it )
		{
			if( (*it).GetState() == SUCCEED )
				b = true;
		}
		if( b )
			state = SUCCEED;
		else
			state = STOPPED;
		FireObjectChange();
	}
};

class SuspendTask : public Thread::StatefulRunnable
{
	int state;
public:
	SuspendTask() : state(INIT) { }
	int GetState() const { return state; }
	void Run()
	{
		state = INIT;
		FireObjectChange();
	}
	void WakeUp()
	{
		state = SUCCEED;
		FireObjectChange();
	}
};

};
};

#endif


