#ifndef __ONLINEGAME_GS_OBJ_MANAGER_H__
#define __ONLINEGAME_GS_OBJ_MANAGER_H__

#include <set>
#include <timer.h>
#include "object.h"

/*
template <int name> bool CheckObjHeartbeat(gobject *obj);
template <> bool CheckObjHeartbeat<0>(gobject * obj)
{
	return (!obj->IsActived() || !obj->imp);
}

template <> bool CheckObjHeartbeat<1>(gobject * obj)
{
	return (!obj->IsActived() || !obj->imp || obj->plane->w_activestate != 1);
}
*/


template <typename T>
struct obj_manager_basic
{
	T *_pool;
	T *_header;
	T *_tail;
	ONET::Thread::Mutex _lock;
	int _count;
	size_t _pool_size;
public:
	obj_manager_basic():_pool(NULL),_header(NULL),_tail(NULL),_count(0),_pool_size(0)
	{}
	~obj_manager_basic()
	{
		//还没有处理
		__PRINTINFO("object manager not free member pool yet\n");
	}

	bool Init(size_t size)
	{
		_pool = (T*)calloc(size,sizeof(T));
		size_t i;
		for(i = 0; i < size; i++)
		{
			_pool[i].pPrev = _pool + (i-1);
			_pool[i].pNext = _pool + (i+1);
		}
		_pool[0].pPrev = NULL;
		_pool[size- 1].pNext = NULL;
		_header = _pool;
		_tail = _pool + (size- 1);
		_pool_size = size;
		return true;
	}

	T * GetPool() const { return _pool;}
	T * GetByIndex(size_t index) const { ASSERT(index < _pool_size); return _pool + index; } 
	size_t GetIndex( T * obj) const { return obj - _pool; }

	T * Alloc()
	{
		ONET::Thread::Mutex::Scoped keeper(_lock);
		if(!_header) return NULL;
		T * pObj = _header;
		_header = (T*)_header->pNext;
		if(_header)
		{
			_header->pPrev = NULL;
		}
		else
		{
			_tail = NULL;
		}
		_count ++;
		keeper.Unlock();
		keeper.Detach();
		ASSERT(pObj->pPiece == NULL && !pObj->IsActived());
		
		pObj->Lock();
		ASSERT(pObj->pPiece == NULL && !pObj->IsActived());
		pObj->SetActive();
		return pObj;
	}
	
	void Free(T * pObj)					//要求传入的结构保持锁定
	{
		ASSERT(pObj->spinlock);
		ASSERT(!pObj->imp);
		//不管如何都清除应该是个好主意
		pObj->Clear();

		ONET::Thread::Mutex::Scoped _keeper(_lock);
		if(!_tail)
		{
			_header = _tail = pObj;
			pObj->pPrev = pObj->pNext = NULL;
		}
		else
		{
			_tail->pNext = pObj;
			pObj->pPrev = _tail;
			pObj->pNext = NULL;
			_tail = pObj;
		}
		_count --;
	}

	inline int GetAllocedCount() const 
	{
		return _count;
	}

	inline size_t GetCapacity() const
	{
		return _pool_size;
	}
};

template <typename OBJECT ,size_t MM_HEARTBEAT_TICK, typename Insertor>
class obj_manager : public obj_manager_basic<OBJECT>, public abase::timer_task, public ONET::Thread::Runnable
{
	ONET::Thread::Mutex _lock_change;
	ONET::Thread::Mutex _lock_heartbeat;

	typedef OBJECT T;
	typedef std::set<OBJECT *> 	OBJECT_SET;
	typedef std::map<OBJECT *,int> 	OBJECT_MAP;

	OBJECT_SET		_obj_set;
	OBJECT_MAP		_change_list;
	typename std::set<OBJECT *>::iterator	_cur_cursor;
	T*  			_cur_obj;
	int			_heart_obj_count;

	inline void DoDelete(T * pObj)
	{
		if(_cur_obj == pObj)
		{
			_cur_cursor = _obj_set.find(_cur_obj);
			++_cur_cursor;
			if(_cur_cursor == _obj_set.end())
			{
				_cur_obj = NULL;
			}
			else
			{
				_cur_obj = *_cur_cursor;
			}
		}

		if(!_obj_set.erase(pObj))
		{
			ASSERT(false);
		}
	}

	inline void DoInsert(T * pObj)
	{
		if(!_obj_set.insert(pObj).second)
		{
			//对多次插入报告错误
			//未来release版应该怎么办？
			ASSERT(false);
		}
	}

	void DoChange()
	{
		ONET::Thread::Mutex::Scoped keeper(_lock_change);
		typename std::map<OBJECT *,int>::const_iterator it = _change_list.begin();
		for(;it != _change_list.end(); ++it)
		{
			if(!it->second) continue;
			ASSERT(it->second == -1 || it->second == 1);
			if(it->second > 0)
			{
				DoInsert(it->first);
				
			}
			else
			{
				DoDelete(it->first);
			}
		}
		_change_list.clear();
	}

	void CollectHeartbeatObject(abase::vector<T *> &list)
	{
		DoChange();
		int size = _obj_set.size();
		_heart_obj_count += size;
		if(size == 0)
		{
			_heart_obj_count = 0;
			_cur_obj = NULL;
			return ;
		}

		typename std::set<OBJECT *>::iterator	end = _obj_set.end();
		if(_cur_obj == NULL)
		{
			_cur_cursor = end;
		}
		else
		{
			_cur_cursor = _obj_set.find(_cur_obj);
		}
#ifdef _DEBUG
		int idle_count = 0;
#endif
		
		list.reserve(size / MM_HEARTBEAT_TICK + 5);
		for(;_heart_obj_count >= (int)MM_HEARTBEAT_TICK; _heart_obj_count -= (int)MM_HEARTBEAT_TICK)
		{
			if(_cur_cursor == end)
			{
				_cur_cursor = _obj_set.begin();
			}

			_cur_obj = *_cur_cursor;

			//将这个对象加入到收集列表中
#ifdef _DEBUG
			idle_count += Insertor::push_back(list,_cur_obj);
#else
			Insertor::push_back(list,_cur_obj);
#endif
			++_cur_cursor;
		}
		if(_cur_cursor != end)
		{
			_cur_obj = *_cur_cursor;
		}
		else
		{
			_cur_obj = NULL;
		}
		
	}
public:
	obj_manager():_cur_obj(NULL),_heart_obj_count(0)
	{}
	~obj_manager()
	{
		//这里不释放timer了，反正不会报错了
		//RemoveTimer();
	}
	
	bool Init(size_t size)
	{
		obj_manager_basic<OBJECT>::Init(size);
		
		//由world来负责定期调用Run，不再使用定时期
		//int rst = SetTimer(g_timer,1,0);
		//ASSERT(rst >=0);
		return true;
	}
	
	void Insert(T * pObj)
	{
	/*
		ONET::Thread::Mutex::Scoped keeper(_lock_insert);
		_ins_list.push_back(pObj);
		*/
		ONET::Thread::Mutex::Scoped keeper(_lock_change);
		_change_list[pObj] ++;
	}

	void Remove(T * pObj)
	{
	/*
		ONET::Thread::Mutex::Scoped keeper(_lock_delete);
		_del_list.push_back(pObj);
		*/
		ONET::Thread::Mutex::Scoped keeper(_lock_change);
		int & val = _change_list[pObj];
		val --;
		ASSERT(val == -1 || val == 0);	//0偶尔能碰到的
	}
	

	void OnHeartbeat()
	{

		ONET::Thread::Mutex::Scoped keeper(_lock_heartbeat);
		abase::vector<T *> list;
		CollectHeartbeatObject(list);
		keeper.Unlock();
		keeper.Detach();
		if(!list.size()) return;

		MSG msg;
		memset(&msg,0,sizeof(msg));
		msg.message = GM_MSG_HEARTBEAT;
		msg.param = MM_HEARTBEAT_TICK/TICK_PER_SEC;

		T** it = list.begin();
		T** end = list.end();
		for(;it != end; ++it)
		{
			gobject * obj = *it;
			obj->Lock();
			//刨除自己的ID  这个判断是否必要?
			if(!obj->IsActived() || !obj->imp || obj->plane->w_activestate != 1) 
	//		if(CheckObjHeartbeat<1>(obj))
			{
				obj->Unlock();
				continue;
			}
			int rst = 0;
			ASSERT(obj->plane);
			rst = obj->imp->DispatchMessage(obj->plane,msg);
			if(!rst)
			{
				ASSERT(obj->spinlock && "这里必须是上锁状态");
				obj->Unlock();
			}
			else
			{
				ASSERT(!obj->spinlock && "没有解开锁，可能是时序问题，但更可能是错误");
			}
		}
	}

	virtual void Run()
	{
		OnHeartbeat();
	}


protected:
	
	void OnTimer(int index,int rtimes)
	{
		ONET::Thread::Pool::AddTask(this);
	}
};

class extern_object_manager: public abase::timer_task, public ONET::Thread::Runnable
{
public:
	struct object_appear
	{
		float body_size;
		int race;
		int faction;
		int level;
		int hp;
		int state;
		int where;
	};
	struct object_entry
	{
		int id;
		A3DVECTOR pos;
		float body_size;
		int race;
		int faction;
		int level;
		int hp;
		int ttl;
		int state;
		int where;
	};
	enum
	{
		NORMAL_TTL = 4
	};

private:
	typedef std::map<int, object_entry> OBJECT_MAP;
	OBJECT_MAP _map;
	ONET::Thread::Mutex _lock;
	int _heartbeat_counter;

public:
	extern_object_manager()
	{
		_heartbeat_counter = 0;
	}

	void Refresh(int id, const A3DVECTOR &pos, const object_appear & obj)	//刷新和加入
	{
		ONET::Thread::Mutex::Scoped keeper(_lock);
		object_entry & ent = _map[id];
		ent.id = id;
		ent.pos = pos;
		ent.body_size = obj.body_size;
		ent.race = obj.race;
		ent.faction = obj.faction;
		ent.level = obj.level;
		ent.hp = obj.hp;
		ent.where = obj.where;
		ent.state = obj.state;
		ent.ttl = NORMAL_TTL;
	}

	void RefreshHP(int id, const A3DVECTOR & pos, int hp)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock);
		OBJECT_MAP::iterator it= _map.find(id);
		if(it != _map.end())
		{
			object_entry & ent = it->second;
			ent.pos = pos;
			ent.hp = hp;
			if(ent.hp == 0)
			{
				ent.state = 1;
			}
			else
			{
				ent.state = 0;
			}
			ent.ttl = NORMAL_TTL;
		}
	}

	void RemoveObject(int id)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock);
		_map.erase(id);
	}

	int QueryServer(int id)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock);
		OBJECT_MAP::iterator it= _map.find(id);
		if(it != _map.end())
		{
			return (int)it->second.where;
		}
		else
		{
			return -1;
		}
	}

	int GetState(int state);

	template <typename INFO>
	bool QueryObject(int id, INFO & info)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock);
		OBJECT_MAP::iterator it= _map.find(id);
		if(it == _map.end())
		{
			return false;
		}
		const object_entry & ent = it->second;
		info.pos = ent.pos;
		info.body_size = ent.body_size;
		info.race = ent.race;
		info.faction = ent.faction;
		info.hp = ent.hp;
		info.mp = 0;
		info.state = GetState(ent.state);
		info.max_hp = ent.hp;
		info.invisible_degree = 0;
		info.anti_invisible_degree = 0;
		info.object_state = 0;
		info.object_state2 = 0;
		return true;
		
	}

	bool Init();
protected:
	friend class world;
	virtual void Run();
	void OnTimer(int index,int rtimes);
private:
	static int GetWorldIndex();// { return world_manager::GetWorldIndex(); }
public:
	void RunTick()
	{	
		int rst = interlocked_increment(&_heartbeat_counter);
		if((rst & 0xFF) == 0)	//每256个tick进行一次Run，合计12.5秒一次
		{
			Run();
		}
	}

	template<int foo>
	static void SendAppearMsg(world *pPlane,gnpc* pNPC,slice * pPiece)
	{
		object_appear app;
		app.body_size = pNPC->body_size;
		app.race = pNPC->tid;
		app.faction = pNPC->base_info.faction;
		app.level= pNPC->base_info.level;
		app.hp= pNPC->base_info.hp;
		app.state = pNPC->IsZombie();
		app.where = 1;
		MSG msg;
		BuildMessage(msg,GM_MSG_EXTERN_OBJECT_APPEAR,XID(GM_TYPE_BROADCAST,-1),pNPC->ID,
				pNPC->pos,0,&app,sizeof(app));
		pPlane->BroadcastSvrMessage(pPiece->slice_range,msg,GRID_SIGHT_RANGE);	//就用视野的广播范围
	}

	template<int foo>
	static void SendAppearMsg(world *pPlane,gmatter* pMatter,slice * pPiece)
	{
		object_appear app;
		app.body_size = pMatter->body_size;
		app.race = pMatter->matter_type;
		app.faction = 0;
		app.level= 0;
		app.hp= 0;
		app.state = 0;
		app.where = 1;
		MSG msg;
		BuildMessage(msg,GM_MSG_EXTERN_OBJECT_APPEAR,XID(GM_TYPE_BROADCAST,-1),pMatter->ID,
				pMatter->pos,0,&app,sizeof(app));
		pPlane->BroadcastSvrMessage(pPiece->slice_range,msg,GRID_SIGHT_RANGE);	//就用视野的广播范围
	}
	
	template<int foo>
	static void SendAppearMsg(world *pPlane, gplayer* pPlayer,slice * pPiece)
	{
		object_appear app;
		app.body_size = pPlayer->body_size;
		app.race = pPlayer->base_info.race;
		app.faction = pPlayer->base_info.faction;
		app.level= pPlayer->base_info.level;
		app.hp= pPlayer->base_info.hp;
		app.state = pPlayer->IsZombie();
		app.where = 1;
		MSG msg;
		BuildMessage(msg,GM_MSG_EXTERN_OBJECT_APPEAR,XID(GM_TYPE_BROADCAST,-1),pPlayer->ID,
				pPlayer->pos,0,&app,sizeof(app));
		pPlane->BroadcastSvrMessage(pPiece->slice_range,msg,GRID_SIGHT_RANGE);	//就用视野的广播范围
	}

	template<int foo>
	static void SendDisappearMsg(world *pPlane,gobject* pObj,slice * pPiece)
	{
		MSG msg;
		BuildMessage(msg,GM_MSG_EXTERN_OBJECT_DISAPPEAR,XID(GM_TYPE_BROADCAST,-1),pObj->ID,
				pObj->pos);
		pPlane->BroadcastSvrMessage(pPiece->slice_range,msg,GRID_SIGHT_RANGE);	//就用视野的广播范围
	}

	template<int foo>
	static void SendRefreshMsg(world *pPlane,gobject * pObj, int hp,slice * pPiece)
	{
		MSG msg;
		BuildMessage(msg,GM_MSG_EXTERN_OBJECT_REFRESH,XID(GM_TYPE_BROADCAST,-1),pObj->ID,
				pObj->pos,hp);
		pPlane->BroadcastSvrMessage(pPiece->slice_range,msg,GRID_SIGHT_RANGE);	//就用视野的广播范围
	}

};

#endif

