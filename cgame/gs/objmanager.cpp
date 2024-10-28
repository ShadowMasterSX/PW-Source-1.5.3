#include "world.h"
#include "worldmanager.h"

int extern_object_manager::GetWorldIndex() 
{ 
	return world_manager::GetWorldIndex(); 
}

	int extern_object_manager::GetState(int state)
	{
		return state ? world::QUERY_OBJECT_STATE_ZOMBIE : world::QUERY_OBJECT_STATE_ACTIVE;
	}
	/*
	template <typename INFO>
	bool extern_object_manager::QueryObject(int id, INFO & info)
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
		info.state = ent.state? world::QUERY_OBJECT_STATE_ZOMBIE : world::QUERY_OBJECT_STATE_ACTIVE;
		info.max_hp = ent.hp;
		info.invisible_degree = 0;
		info.anti_invisible_degree = 0;
		info.object_state = 0;
		info.object_state2 = 0;
		return true;
		
	}*/

	//template <world::object_info>
	/*bool extern_object_manager::QueryObject(int id, world::object_info & info)
	{
		ONET::Thread::Mutex::Scoped keeper(_lock);
		OBJECT_MAP::iterator it= _map.find(id);
		if(it == _map.end())
		{
			return false;
		}
		const object_entry & ent = it->second;
		
		int level;
		int mafia_id;
		
		info.pos = ent.pos;
		info.body_size = ent.body_size;
		info.race = ent.race;
		info.faction = ent.faction;
		info.hp = ent.hp;
		info.mp = 0;
		info.state = ent.state? world::QUERY_OBJECT_STATE_ZOMBIE : world::QUERY_OBJECT_STATE_ACTIVE;
		info.max_hp = ent.hp;
		info.invisible_degree = 0;
		info.anti_invisible_degree = 0;
		info.object_state = 0;
		info.object_state2 = 0;
		return true;
		
	}*/

bool 
extern_object_manager::Init()
{
	int rst = SetTimer(g_timer,20*15,0);
	ASSERT(rst >=0);
	return true;

}

void 
extern_object_manager::Run()
{
	//每10秒一次heartbeat
	//估计一个服务的量为1000左右
	ONET::Thread::Mutex::Scoped keeper(_lock);
	OBJECT_MAP::iterator it = _map.begin();
	for(;it != _map.end(); )
	{
		object_entry & ent = it->second;
		if(--ent.ttl <= 0) 
		{
			__PRINTF("object %x removed\n",ent.id);
			_map.erase(it++); 
		}
		else
		{
			++it;
		}
	}
}

void 
extern_object_manager::OnTimer(int index,int rtimes)
{
	ONET::Thread::Pool::AddTask(this);
}

