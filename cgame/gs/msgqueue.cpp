#include <stdlib.h>
#include "msgqueue.h"
#include "world.h"


void MsgQueue::Send(world * plane)
{
	int tmpcount = 0;
	{
		MSGQUEUE::iterator it = _queue.begin();
		MSGQUEUE::iterator end = _queue.end();
		tmpcount += (end - it);
		for(;it != end; ++it)
		{
			SendMessage(plane,**it);
		}
	}
	{
		MultiCast **it = _multi_queue.begin();
		MultiCast **end = _multi_queue.end();
		//发送消息依次
		for(;it != end; ++it)
		{
			(*it)->Send(plane);
		}
	}

	{
		IDMultiCast **it = _id_multi_queue.begin();
		IDMultiCast **end = _id_multi_queue.end();
		//发送消息依次
		for(;it != end; ++it)
		{
			(*it)->Send(plane);
		}
	}

	Clear();
}

void MsgQueue::MultiCast::Send(world * pPlane)
{
	ObjList::iterator it = _list.begin();
	ObjList::iterator end = _list.end();
	//发送消息依次
	int ttl = _msg->ttl;
	for(;it != end; ++it)
	{
		gobject * obj = *it;
		//刨除自己的ID  这个判断是否必要?
		if(!obj->IsActived() || obj->ID.id == _msg->source.id) {
			continue;
		}
		pPlane->DispatchMessage(obj,*_msg);
		_msg->ttl = ttl;
	}
}

void MsgQueue::IDMultiCast::Send(world * pPlane)
{
	IDList::iterator it = _id_list.begin();
	IDList::iterator end = _id_list.end();
	//先扫描一遍
	for(;it != end; ++it)
	{
		if(!it->IsValid())
		{
			GLog::log(GLOG_ERR,"IDMultiCast::Send 消息%d中存在错误的目标(%d,%d)",_msg->message,it->type,it->id);
		}
	}
	it = _id_list.begin();
	
	//发送消息依次
	int ttl = _msg->ttl;
	for(;it != end; ++it)
	{
		_msg->target = *it;
		SendMessage(pPlane,*_msg);
		_msg->ttl = ttl;
	}
}

void MsgQueue::Run()
{
	Send(_plane);
	delete this;
}

void MsgQueue::OnTimer(int index,int rtimes)
{
	ASSERT(index != _timer_index );
	ONET::Thread::Pool::AddTask(this);
}


void 
MsgQueueList::OnTimer(int index,int rtimes)
{
	if(_cur_queue_count > 0)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock_cur);
		if(!_cur_queue.IsEmpty())
		{
			_SEND_CUR_QUEUE();
		}
	}

	{
		ONET::Thread::Mutex::Scoped keeper(_lock);
		if(_list[_offset])
		{
			_list[_offset]->AddTask(_plane);
			_list[_offset] = NULL;
		}
		_offset ++;
		ASSERT(_offset <= MAX_MESSAGE_LATENCY);
		if(_offset >= MAX_MESSAGE_LATENCY) _offset = 0;
	}

}

MsgQueueList::~MsgQueueList()
{
}

//------------------------------------------------------------------------------------------


MsgQueue2::MultiCast::MultiCast(world * plane , ObjList &list, const MSG &msg): _msg(DupeMessage(msg)),_plane(plane),_world_index(plane->w_index_in_man)
{	
	//注意，这里更改了传入的参数 
	_list.swap(list);
}

MsgQueue2::IDMultiCast::IDMultiCast(world * plane,IDList &list, const MSG & msg):_msg(DupeMessage(msg)),_plane(plane),_world_index(plane->w_index_in_man)
{
	//注意，这里更改了传入的参数 
	_id_list.swap(list);
}

MsgQueue2::IDMultiCast::IDMultiCast(world *plane,const XID *first, const XID * last, const MSG & msg):_msg(DupeMessage(msg)),_plane(plane),_world_index(plane->w_index_in_man)
{
	ASSERT((int)(last - first) > 0);
	_id_list.reserve(last - first);
	for(;first != last; ++first)
	{
		_id_list.push_back(*first);
	}
}

MsgQueue2::IDMultiCast::IDMultiCast(world * plane,size_t count,const int * playerlist, const MSG & msg):_msg(DupeMessage(msg)),_plane(plane),_world_index(plane->w_index_in_man)
{
	_id_list.reserve(count);
	XID id(GM_TYPE_PLAYER,-1);
	for(size_t i = 0; i <count ; i ++,playerlist ++)
	{
		id.id = * playerlist;
		_id_list.push_back(id);
	}
}

void MsgQueue2::Send()
{
	int tmpcount = 0;
	{
		MSGQUEUE::iterator it = _queue.begin();
		MSGQUEUE::iterator end = _queue.end();
		tmpcount += (end - it);
		for(;it != end; ++it)
		{
			const MSG & msg = *(it->msg);
			int world_index = it->world_index;
			if(world_index < 0 
				|| world_manager::GetInstance()->GetWorldByIndex(world_index) != it->pPlane)
			{
				__PRINTF("a:抛弃了不符合的消息msg %d from %d to %d ,world_index %d, plane: %p\n",msg.message, msg.source.id,msg.target.id,world_index, it->pPlane);
				continue;
			}
			SendMessage(it->pPlane, msg);
		}
	}
	{
		MultiCast **it = _multi_queue.begin();
		MultiCast **end = _multi_queue.end();
		//发送消息依次
		for(;it != end; ++it)
		{
			(*it)->Send();
		}
	}

	{
		IDMultiCast **it = _id_multi_queue.begin();
		IDMultiCast **end = _id_multi_queue.end();
		//发送消息依次
		for(;it != end; ++it)
		{
			(*it)->Send();
		}
	}

	Clear();
}

void MsgQueue2::MultiCast::Send()
{
	if(_world_index < 0 || world_manager::GetInstance()->GetWorldByIndex(_world_index) != _plane)
	{
		__PRINTF("a:抛弃了不符合的消息msg %d from %d to %d\n",_msg->message, _msg->source.id,_msg->target.id);
		return; //世界已经不再激活
	}

	ObjList::iterator it = _list.begin();
	ObjList::iterator end = _list.end();
	//发送消息依次
	int ttl = _msg->ttl;
	for(;it != end; ++it)
	{
		gobject * obj = *it;
		//刨除自己的ID  这个判断是否必要?
		if(!obj->IsActived() || obj->ID.id == _msg->source.id) {
			continue;
		}
		_plane->DispatchMessage(obj,*_msg);
		_msg->ttl = ttl;
	}
}

void MsgQueue2::IDMultiCast::Send()
{
	if(_world_index < 0 || world_manager::GetInstance()->GetWorldByIndex(_world_index) != _plane)
	{
		__PRINTF("b:抛弃了不符合的消息msg %d from %d to %d\n",_msg->message, _msg->source.id,_msg->target.id);
		return; //世界已经不再激活
	}
	
	IDList::iterator it = _id_list.begin();
	IDList::iterator end = _id_list.end();
	//发送消息依次
	int ttl = _msg->ttl;
	for(;it != end; ++it)
	{
		_msg->target = *it;
		SendMessage(_plane,*_msg);
		_msg->ttl = ttl;
	}
}

void MsgQueue2::Run()
{
	Send();
	delete this;
}

void MsgQueue2::OnTimer(int index,int rtimes)
{
	ASSERT(index != _timer_index );
	ONET::Thread::Pool::AddTask(this);
}

void 
MsgQueue2::AddMsg(world * plane,const MSG & msg)
{
	_queue.push_back(msg_t(plane->w_index_in_man,plane,DupeMessage(msg)));
}


void 
MsgQueueList2::OnTimer(int index,int rtimes)
{
	if(_cur_queue_count > 0)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock_cur);
		if(!_cur_queue.IsEmpty())
		{
			_SEND_CUR_QUEUE();
		}
	}

	{
		ONET::Thread::Mutex::Scoped keeper(_lock);
		if(_list[_offset])
		{
			_list[_offset]->AddTask();
			_list[_offset] = NULL;
		}
		_offset ++;
		ASSERT(_offset <= MAX_MESSAGE_LATENCY);
		if(_offset >= MAX_MESSAGE_LATENCY) _offset = 0;
	}

}

MsgQueueList2::~MsgQueueList2()
{
}

