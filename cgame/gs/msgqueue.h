#ifndef __ONLINEGAME_GS_MSGQUEUE_H__
#define __ONLINEGAME_GS_MSGQUEUE_H__
#include <ASSERT.h>
#include <threadpool.h>
#include <hashtab.h>
#include <vector.h>
#include <common/types.h>
#include <common/message.h>
#include <timer.h>
#include <amemobj.h>
#include <amemory.h>


#include "config.h"

class world;
struct gobject;
//发送消息队列
class MsgQueue : public ONET::Thread::Runnable, public abase::timer_task , public abase::ASmallObject
{
	typedef abase::vector<gobject*,abase::fast_alloc<> > ObjList;
	typedef abase::vector<XID,abase::fast_alloc<> >  IDList;
	struct MultiCast : abase::ASmallObject
	{
		ObjList _list;
		MSG * _msg;
		MultiCast(ObjList &list, const MSG &msg): _msg(DupeMessage(msg))
		{	
			//注意，这里更改了传入的参数 
			_list.swap(list);
		}

		~MultiCast()
		{
			FreeMessage(_msg);
		}
		void Send(world * pPlane);
	};

	struct IDMultiCast : abase::ASmallObject
	{
		IDList	_id_list;
		MSG * _msg;

		IDMultiCast(IDList &list, const MSG & msg):_msg(DupeMessage(msg))
		{
			//注意，这里更改了传入的参数 
			_id_list.swap(list);
		}

		IDMultiCast(const XID *first, const XID * last, const MSG & msg):_msg(DupeMessage(msg))
		{
			ASSERT((int)(last - first) > 0);
			_id_list.reserve(last - first);
			for(;first != last; ++first)
			{
				ASSERT(!(*first).IsErrorType());
				_id_list.push_back(*first);
			}
		}

		IDMultiCast(size_t count,const int * playerlist, const MSG & msg):_msg(DupeMessage(msg))
		{
			_id_list.reserve(count);
			XID id(GM_TYPE_PLAYER,-1);
			for(size_t i = 0; i <count ; i ++,playerlist ++)
			{
				id.id = * playerlist;
				_id_list.push_back(id);
			}
		}

		~IDMultiCast()
		{
			FreeMessage(_msg);
		}
		void Send(world * pPlane);
	};
	typedef abase::vector<MSG*,abase::fast_alloc<> > MSGQUEUE;
	typedef abase::vector<MultiCast*,abase::fast_alloc<> > MULTICASTQUEUE;
	typedef abase::vector<IDMultiCast*,abase::fast_alloc<> > IDMULTICASTQUEUE;
	MSGQUEUE _queue;
	MULTICASTQUEUE _multi_queue;
	IDMULTICASTQUEUE _id_multi_queue;

	template <typename WORLD>
	static inline void SendMessage(WORLD *pPlane,const MSG & msg)
	{	
		pPlane->DispatchMessage(msg);
	}
public:
	world * _plane;
public:
	void Swap(MsgQueue & queue)
	{
		_queue.swap(queue._queue);
		_multi_queue.swap(queue._multi_queue);
		_id_multi_queue.swap(queue._id_multi_queue);
		world * tmp = _plane;
		_plane = queue._plane;
		queue._plane = tmp;
	}
	
	MsgQueue():_plane(0)
	{
	}
	~MsgQueue()
	{
		if(_queue.size() || _multi_queue.size() || _id_multi_queue.size()) Clear();
	}
	void AddMsg(const MSG & msg)
	{
		_queue.push_back(DupeMessage(msg));
	}

	void AddMultiMsg(ObjList & list,const MSG & msg)
	{
		_multi_queue.push_back(new MultiCast(list,msg));
	}

	void AddMultiMsg(const XID * first, const XID * last, const MSG & msg)
	{
		_id_multi_queue.push_back(new IDMultiCast(first,last,msg));
	}

	void AddPlayerMultiMsg(size_t count, const int *player_list, const MSG & msg)
	{
		_id_multi_queue.push_back(new IDMultiCast(count,player_list,msg));
	}

	bool IsEmpty() 
	{
		return _queue.empty() && _multi_queue.empty() && _id_multi_queue.empty();
	}

	void Clear()
	{
		MSGQUEUE::iterator it = _queue.begin();
		MSGQUEUE::iterator end = _queue.end();
		for(;it != end; ++it)
		{
			FreeMessage(*it);
		}
		_queue.clear();
		abase::clear_ptr_vector(_multi_queue);
		abase::clear_ptr_vector(_id_multi_queue);
	}
	
	void Send(world * plane);
	void AddTask(world * plane)
	{
		_plane = plane;
		ONET::Thread::Pool::AddTask(this);
	}


	//延迟若干个tick再发送消息队列
	void AddTask(world * plane, abase::timer & tm ,int delay)
	{
		//此种方案已经不再使用
		ASSERT(false);
		_plane = plane;
		if(SetTimer(tm,delay,1) < 0)
		{
			//如果没有timer了，那么就直接立刻发送，同时要做日志
			ONET::Thread::Pool::AddTask(this);
		}
	}
	
	virtual void OnTimer(int index,int rtimes);
	virtual void Run();
};

class MsgQueueList : public abase::timer_task
{
	typedef abase::vector<gobject*,abase::fast_alloc<> > ObjList;
	MsgQueue * 		_list[MAX_MESSAGE_LATENCY];
	MsgQueue 		_cur_queue;
	int 	   		_offset;
	world *			_plane;
	ONET::Thread::Mutex 	_lock;
	ONET::Thread::Mutex 	_lock_cur;
	ONET::Thread::Mutex 	_lock_hb;
	size_t			_cur_queue_count;
	enum
	{	
		SIZE = MAX_MESSAGE_LATENCY
	};

	MsgQueue * GetQueue(int target)
	{
		if(_list[target] == NULL)
		{
			_list[target] = new MsgQueue();
		}
		return _list[target];
	}
	enum 
	{
		TASK_SIZE = 256
	};

	inline void _SEND_CUR_QUEUE()
	{
		MsgQueue * pQueue = new MsgQueue;
		_cur_queue.Swap(*pQueue);
		pQueue->AddTask(_plane);
		_cur_queue_count = 0;
	}
	inline void _TEST_SEND_CUR_QUEUE()
	{	
		if(++_cur_queue_count < TASK_SIZE) return;
		_SEND_CUR_QUEUE();
	}
public:	
	MsgQueueList():_offset(0),_plane(NULL)
	{
		_cur_queue_count = 0;
		for(int i = 0; i< SIZE; i++)
		{
			_list[i] = NULL;
		}
	}
	~MsgQueueList();
	
	void Init(world *pPlane,abase::timer & tm)
	{
		_plane = pPlane;
		//不再设置定时期了，由world来负责调用run
		//	SetTimer(tm,1,0);
	}

	virtual void OnTimer(int index,int rtimes);
	void AddMsg(const MSG & msg)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock_cur);
		_cur_queue.AddMsg(msg);
		_TEST_SEND_CUR_QUEUE();
	}

	void AddMultiMsg(abase::vector<gobject*,abase::fast_alloc<> > &list, const MSG & msg)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock_cur);
		_cur_queue.AddMultiMsg(list,msg);
		_TEST_SEND_CUR_QUEUE();
		return ;
	}

	void AddMultiMsg(const XID * first, const XID * last ,const MSG & msg)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock_cur);
		_cur_queue.AddMultiMsg(first,last,msg);
		_TEST_SEND_CUR_QUEUE();
		return ;
	}

	void AddPlayerMultiMsg(size_t count , const int * player_list, const MSG & msg)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock_cur);
		_cur_queue.AddPlayerMultiMsg(count,player_list,msg);
		_TEST_SEND_CUR_QUEUE();
		return ;
	}

	void SendCurQueue()
	{
		ONET::Thread::Mutex::Scoped keeper(_lock_cur);
		if(_cur_queue.IsEmpty()) return;
		MsgQueue queue;
		queue.Swap(_cur_queue);
		keeper.Unlock();
		keeper.Detach();
		queue.Send(_plane);
	}

	void AddMsg(const MSG & msg,size_t latency)
	{
		ASSERT(latency < MAX_MESSAGE_LATENCY);
		ONET::Thread::Mutex::Scoped keeper(_lock);
		int target = _offset + latency;
		if(target >= SIZE) target %= SIZE;
		GetQueue(target)->AddMsg(msg);
		return ;
	}

	void AddMultiMsg(abase::vector<gobject*,abase::fast_alloc<> > &list, const MSG & msg,size_t latency)
	{
		ASSERT(latency < MAX_MESSAGE_LATENCY);
		ONET::Thread::Mutex::Scoped keeper(_lock);
		int target = _offset + latency;
		if(target >= SIZE) target %= SIZE;
		GetQueue(target)->AddMultiMsg(list,msg);
		return ;
	}
};

//-----------------------------------------------------------------------------------------
class MsgQueue2 : public ONET::Thread::Runnable, public abase::timer_task , public abase::ASmallObject
{
	typedef abase::vector<gobject*,abase::fast_alloc<> > ObjList;
	typedef abase::vector<XID,abase::fast_alloc<> >  IDList;
	struct MultiCast : abase::ASmallObject
	{
		ObjList _list;
		MSG * _msg;
		world * _plane;
		int _world_index;
		MultiCast(world * plane , ObjList &list, const MSG &msg);
		~MultiCast()
		{
			FreeMessage(_msg);
		}
		void Send();
	};

	struct IDMultiCast : abase::ASmallObject
	{
		IDList	_id_list;
		MSG * _msg;
		world * _plane;
		int _world_index;

		IDMultiCast(world * plane,IDList &list, const MSG & msg);
		IDMultiCast(world *plane,const XID *first, const XID * last, const MSG & msg);
		IDMultiCast(world * plane,size_t count,const int * playerlist, const MSG & msg);

		~IDMultiCast()
		{
			FreeMessage(_msg);
		}
		void Send();
	};

	struct  msg_t
	{
		int  world_index;
		world * pPlane;
		MSG * msg;
		msg_t(int index, world * plane, MSG *__msg):world_index(index),pPlane(plane),msg(__msg)
		{
		}
		msg_t()
		{
		}
	};
	
	typedef abase::vector<msg_t ,abase::fast_alloc<> > MSGQUEUE;
	typedef abase::vector<MultiCast*,abase::fast_alloc<> > MULTICASTQUEUE;
	typedef abase::vector<IDMultiCast*,abase::fast_alloc<> > IDMULTICASTQUEUE;
	MSGQUEUE _queue;
	MULTICASTQUEUE _multi_queue;
	IDMULTICASTQUEUE _id_multi_queue;

	template <typename WORLD>
	static inline void SendMessage(WORLD *pPlane,const MSG & msg)
	{	
		pPlane->DispatchMessage(msg);
	}
public:
	void Swap(MsgQueue2 & queue)
	{
		_queue.swap(queue._queue);
		_multi_queue.swap(queue._multi_queue);
		_id_multi_queue.swap(queue._id_multi_queue);
	}
	
	MsgQueue2()
	{
	}
	~MsgQueue2()
	{
		if(_queue.size() || _multi_queue.size() || _id_multi_queue.size()) Clear();
	}
	void AddMsg(world * plane,const MSG & msg);
	void AddMultiMsg(world * plane,ObjList & list,const MSG & msg)
	{
		_multi_queue.push_back(new MultiCast(plane,list,msg));
	}

	void AddMultiMsg(world * plane, const XID * first, const XID * last, const MSG & msg)
	{
		_id_multi_queue.push_back(new IDMultiCast(plane,first,last,msg));
	}

	void AddPlayerMultiMsg(world * plane,size_t count, const int *player_list, const MSG & msg)
	{
		_id_multi_queue.push_back(new IDMultiCast(plane,count,player_list,msg));
	}

	bool IsEmpty() 
	{
		return _queue.empty() && _multi_queue.empty() && _id_multi_queue.empty();
	}

	void Clear()
	{
		MSGQUEUE::iterator it = _queue.begin();
		MSGQUEUE::iterator end = _queue.end();
		for(;it != end; ++it)
		{
			FreeMessage(it->msg);
		}
		_queue.clear();
		abase::clear_ptr_vector(_multi_queue);
		abase::clear_ptr_vector(_id_multi_queue);
	}
	
	void Send();
	void AddTask()
	{
		ONET::Thread::Pool::AddTask(this);
	}


	//延迟若干个tick再发送消息队列
	void AddTask(abase::timer & tm ,int delay)
	{
		//此种方案已经不再使用
		ASSERT(false);
		if(SetTimer(tm,delay,1) < 0)
		{
			//如果没有timer了，那么就直接立刻发送
			ONET::Thread::Pool::AddTask(this);
		}
	}
	
	virtual void OnTimer(int index,int rtimes);
	virtual void Run();
};

class MsgQueueList2 : public abase::timer_task
{
	typedef abase::vector<gobject*,abase::fast_alloc<> > ObjList;
	MsgQueue2 * 		_list[MAX_MESSAGE_LATENCY];
	MsgQueue2 		_cur_queue;
	int 	   		_offset;
	ONET::Thread::Mutex 	_lock;
	ONET::Thread::Mutex 	_lock_cur;
	ONET::Thread::Mutex 	_lock_hb;
	size_t			_cur_queue_count;
	enum
	{	
		SIZE = MAX_MESSAGE_LATENCY
	};

	MsgQueue2 * GetQueue(int target)
	{
		if(_list[target] == NULL)
		{
			_list[target] = new MsgQueue2();
		}
		return _list[target];
	}
	enum 
	{
		TASK_SIZE = 256
	};
	
	inline void _SEND_CUR_QUEUE()
	{
		MsgQueue2 * pQueue = new MsgQueue2;
		_cur_queue.Swap(*pQueue);
		pQueue->AddTask();
		_cur_queue_count = 0;
	}
	inline void _TEST_SEND_CUR_QUEUE()
	{	
		if(++_cur_queue_count < TASK_SIZE) return;
		_SEND_CUR_QUEUE();
	}
public:	
	MsgQueueList2():_offset(0)
	{
		_cur_queue_count = 0;
		for(int i = 0; i< SIZE; i++)
		{
			_list[i] = NULL;
		}
	}
	~MsgQueueList2();
	
	void Init()
	{
		//不再设置定时期了，由world来负责调用run
		//	SetTimer(tm,1,0);
	}

	virtual void OnTimer(int index,int rtimes);
	void AddMsg(world * plane,const MSG & msg)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock_cur);
		_cur_queue.AddMsg(plane,msg);
		_TEST_SEND_CUR_QUEUE();
	}

	void AddMultiMsg(world * plane,abase::vector<gobject*,abase::fast_alloc<> > &list, const MSG & msg)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock_cur);
		_cur_queue.AddMultiMsg(plane,list,msg);
		_TEST_SEND_CUR_QUEUE();
		return ;
	}

	void AddMultiMsg(world * plane,const XID * first, const XID * last ,const MSG & msg)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock_cur);
		_cur_queue.AddMultiMsg(plane,first,last,msg);
		_TEST_SEND_CUR_QUEUE();
		return ;
	}

	void AddPlayerMultiMsg(world * plane,size_t count , const int * player_list, const MSG & msg)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock_cur);
		_cur_queue.AddPlayerMultiMsg(plane,count,player_list,msg);
		_TEST_SEND_CUR_QUEUE();
		return ;
	}

	void SendCurQueue()
	{
		ONET::Thread::Mutex::Scoped keeper(_lock_cur);
		if(_cur_queue.IsEmpty()) return;
		MsgQueue2 queue;
		queue.Swap(_cur_queue);
		keeper.Unlock();
		keeper.Detach();
		queue.Send();
	}

	void AddMsg(world * plane,const MSG & msg,size_t latency)
	{
		ASSERT(latency < MAX_MESSAGE_LATENCY);
		ONET::Thread::Mutex::Scoped keeper(_lock);
		int target = _offset + latency;
		if(target >= SIZE) target %= SIZE;
		GetQueue(target)->AddMsg(plane,msg);
		return ;
	}

	void AddMultiMsg(world * plane,abase::vector<gobject*,abase::fast_alloc<> > &list, const MSG & msg,size_t latency)
	{
		ASSERT(latency < MAX_MESSAGE_LATENCY);
		ONET::Thread::Mutex::Scoped keeper(_lock);
		int target = _offset + latency;
		if(target >= SIZE) target %= SIZE;
		GetQueue(target)->AddMultiMsg(plane,list,msg);
		return ;
	}
};
#endif

