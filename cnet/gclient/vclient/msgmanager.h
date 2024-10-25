#ifndef __VCLIENT_MSGQUEUE_H__
#define __VCLIENT_MSGQUEUE_H__

#include <list>
#include "spinlock.h"
#include "ASSERT.h"
#include "thread.h"

class MSG;
class MsgQueue : public GNET::Thread::Runnable
{
	std::list<MSG *> _list;
	
public:
	MsgQueue(){}
	~MsgQueue();
	
	void AddMessage(MSG * msg)
	{
		_list.push_back(msg);
	}

	void DispatchMessage()
	{
		GNET::Thread::Pool::AddTask(this);		
	}

	virtual void Run();
};


class MsgManager
{
	SpinLock _lock_cur;	
	size_t _cur_size;
	MsgQueue * _cur_queue;

	enum
	{
		CRITICAL_SIZE = 32,
	};

public:
	MsgManager():_cur_size(0),_cur_queue(new MsgQueue()){}
	~MsgManager(){ if(_cur_queue) delete _cur_queue; }

	void PostMessage(MSG * msg)
	{
		SpinLock::Scoped l(_lock_cur);
		_cur_queue->AddMessage(msg);		
		if(++_cur_size >= CRITICAL_SIZE)
		{
			_cur_queue->DispatchMessage();
			_cur_size = 0;			
			_cur_queue = new MsgQueue();
		}
	}

	void Tick()
	{
		SpinLock::Scoped l(_lock_cur);
		if(_cur_size)
		{
			_cur_queue->DispatchMessage();
			_cur_size = 0;			
			_cur_queue = new MsgQueue();
		}
	}
};


#endif
