#include "../world.h"
#include "../player_imp.h"
#include "../aei_filter.h"
#include "parallel_world_manager.h"


world_message_handler * parallel_world_manager::CreateMessageHandler()
{
	return new parallel_world_message_handler(this);
}

void parallel_world_manager::Heartbeat()
{
	_msg_queue.OnTimer(0,100);
	world_manager::Heartbeat();
	size_t ins_count = _max_active_index;
	for(size_t i = 0; i < ins_count ; i ++)
	{
		if(_planes_state[i] == 0)
		{
			continue;
		}
		world * pPlane = _cur_planes[i];
		if(!pPlane) continue;
		pPlane->RunTick();
	}

	mutex_spinlock(&_heartbeat_lock);
	
	if((++_heartbeat_counter) > TICK_PER_SEC*HEARTBEAT_CHECK_INTERVAL)
	{
		//每10秒检验一次
		for(size_t i = 0; i < ins_count ; i ++)
		{
			if(_planes_state[i] == 0) continue;	//空世界
			world * pPlane = _cur_planes[i];
			if(!pPlane) continue;
			if(pPlane->w_obsolete)
			{
				//处于等待废除状态
				if(pPlane->w_player_count || !WorldCapacityHigh())
				{
					pPlane->w_obsolete = 0;
				}
				else
				{
					if((pPlane->w_obsolete += HEARTBEAT_CHECK_INTERVAL) > _idle_time)
					{
						//没有玩家并且时间超时了，则释放世界
						FreeWorld(pPlane,i);
					}
				}
			}
			else
			{
				if(!pPlane->w_player_count && WorldCapacityHigh())
				{
					pPlane->w_obsolete = 1;
				}
			}
		}
		_heartbeat_counter = 0;

		//进行冷却列表的处理
		RegroupCoolDownWorld();

		if(WorldCapacityLow() && _active_plane_count + 1 < _planes_capacity)
		{
			AutoAllocWorld();
		}
#if 0
		__PRINTF("--------------------------------------------------------\n");
		_idle_time = 60;	//set idle time 60s for test
		ins_count = _max_active_index;
		for(size_t i = 0; i < ins_count ; i ++)
		{
			if(_planes_state[i] == 0)
			{
				continue;
			}
			world * pPlane = _cur_planes[i];
			if(!pPlane) continue;
			__PRINTF("-----\t\tWORLD %d\tpcount %d\tobsolete %d\n",i,pPlane->w_player_count,pPlane->w_obsolete);
		}
		__PRINTF("--------------------------------------------------------\n");
#endif
	}

	if((++_heartbeat_counter2) > TICK_PER_SEC*HEARTBEAT_CHECK_INTERVAL)
	{
		//如果世界池的容量不足，则进行重新创建处理
		FillWorldPool();

		_heartbeat_counter2 = 0;
	}

	mutex_spinunlock(&_heartbeat_lock);
}

void parallel_world_manager::SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey)
{
	if(world_manager::NeedVisa())
		pImp->_filters.AddFilter(new aecv_filter(pImp,FILTER_INDEX_CHECK_VISA));
}

int parallel_world_manager::PlayerSwitchWorld(gplayer * pPlayer, const instance_hash_key & hkey)
{
	ASSERT(pPlayer->spinlock && "Player必须为加锁状态");
	gplayer_imp * pImp = (gplayer_imp *)(pPlayer->imp);
	world * pPlane = pImp->_plane;
	
	//检查目标world是否可进入
	world * new_pPlane = NULL;
	{
		spin_autolock keeper(_key_lock);
		int * pTmp = _key_map.nGet(hkey);
		if(!pTmp) return S2C::ERR_PARALLEL_WORLD_NOT_EXIST;
		new_pPlane = _cur_planes[*pTmp];
		ASSERT(new_pPlane);
		if(new_pPlane == pPlane)
		{
			__PRINTF("目标平行世界是自己,无法切换\n");
			return S2C::ERR_CANNOT_SWITCH_IN_PARALLEL_WORLD;
		}
		if(new_pPlane->w_player_count >= _player_limit_per_instance)
		{
			__PRINTF("目标平行世界玩家已满\n");
			return S2C::ERR_TOO_MANY_PLAYER_IN_PARALLEL_WORLD;
		}
	}
	if(new_pPlane->FindPlayer(pPlayer->ID.id) >= 0)
	{
		ASSERT(false);
		GLog::log(GLOG_ERR, "玩家(id=%d)当前处于world(widx=%d createtime=%d)中，但同时存在于world2(widx=%d, createtime=%d)的查询表中", 
				pPlayer->ID.id, pPlane->w_plane_index, pPlane->w_create_timestamp, new_pPlane->w_plane_index, new_pPlane->w_create_timestamp);
		return S2C::ERR_CANNOT_SWITCH_IN_PARALLEL_WORLD;
	}
	//向周围玩家发送消失消息
	pImp->_runner->disappear();
		
	pImp->PlayerLeaveParallelWorld();
	//从原world的各种映射表中删除玩家
	if(pPlayer->pPiece)
	{
		pPlane->RemovePlayer(pPlayer);
	}
	pPlane->UnmapPlayer(pPlayer->ID.id);
	pPlane->DetachPlayer(pPlayer);
	//新world各种映射表中增加玩家
	ASSERT(pPlayer->pPiece == NULL && pPlayer->plane == NULL);
	new_pPlane->AttachPlayer(pPlayer);
	if(!new_pPlane->MapPlayer(pPlayer->ID.id,new_pPlane->GetPlayerIndex(pPlayer)))
	{
		ASSERT(false);	
	}
	SetPlayerWorldIdx(pPlayer->ID.id,new_pPlane->w_plane_index);
	new_pPlane->InsertPlayer(pPlayer);
	//重置imp中的pPlane
	pImp->ResetPlane(new_pPlane);

	pImp->PlayerEnterParallelWorld();
	//重新发送玩家数据
	pImp->_runner->notify_pos(pPlayer->pos);
	pImp->_runner->begin_transfer();
	pImp->_runner->enter_world();
	pImp->_runner->end_transfer();

	GLog::log(GLOG_INFO,"用户%d(%d,%d)进行了平行世界切换(%d->%d)",
			pPlayer->ID.id, pPlayer->cs_index,pPlayer->cs_sid,pPlane->w_plane_index,new_pPlane->w_plane_index);
	return 0;
}

void parallel_world_manager::PlayerQueryWorld(gplayer * pPlayer)
{
	packet_wrapper h1(64);
	using namespace S2C;
	CMD::Make<CMD::parallel_world_info>::From(h1, _world_tag, _active_plane_count);
	size_t ins_count = _max_active_index;
	for(size_t i = 0; i < ins_count ; i ++)
	{
		if(_planes_state[i] == 0) continue;
		world * pPlane = _cur_planes[i];
		if(!pPlane) continue;
		CMD::Make<CMD::parallel_world_info>::Add(h1, pPlane->w_ins_key, 1.0f*pPlane->w_player_count/_player_limit_per_instance);
	}
	send_ls_msg(pPlayer, h1);
}

int parallel_world_manager::CheckPlayerSwitchRequest(const XID & who,const instance_key * ikey,const A3DVECTOR & pos,int ins_timer)
{
	instance_hash_key key;
	TransformInstanceKey(ikey->target,key);
	world *pPlane = NULL;
	spin_autolock keeper(_key_lock);
	int * pTmp = _key_map.nGet(key);
	if(pTmp)
	{
		pPlane = _cur_planes[*pTmp];
		ASSERT(pPlane);
		if(pPlane->w_player_count < _player_limit_per_instance)
		{
			return 0;
		}
	}
	//玩家进入场景时未指定世界或指定的世界人数满，试图为玩家选择一个世界
	int world_index;
	pPlane = GetAvailabelWorldRandom(world_index);
	if(pPlane) ModifyInstanceKey(const_cast<instance_key::key_essence &>(ikey->target), pPlane->w_ins_key);
	
	return (pPlane ? 0 : S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE);
}

world * parallel_world_manager::GetWorldInSwitch(const instance_hash_key & ikey,int & world_index,int )
{
	spin_autolock keeper(_key_lock);
	world *pPlane = NULL;
	world_index = -1;
	int * pTmp = _key_map.nGet(ikey);
	if(pTmp)
	{
		//存在这样的世界 
		world_index = *pTmp;;
		pPlane = _cur_planes[world_index];
		ASSERT(pPlane);
		pPlane->w_obsolete = 0;
	}
	if(world_index < 0) return NULL;
	return pPlane;
}

world * parallel_world_manager::GetWorldOnLogin(const instance_hash_key & key,int & world_index)
{
	spin_autolock keeper(_key_lock);
	world *pPlane = NULL;
	world_index = -1;
	int * pTmp = _key_map.nGet(key);
	if(pTmp)
	{
		world_index = *pTmp;
		pPlane = _cur_planes[world_index];
		ASSERT(pPlane);
		if(pPlane->w_player_count < _player_limit_per_instance)
		{
			pPlane->w_obsolete = 0;
			return pPlane;
		}
	}
	//玩家上次登陆时的世界已消失或世界人数已满，试图为玩家选择一个世界
	pPlane = GetAvailabelWorldRandom(world_index);
	if(pPlane) pPlane->w_obsolete = 0;
	
	if(world_index < 0) return NULL;
	return pPlane;
}

world * parallel_world_manager::GetFirstAvailabelWorld(int & world_index)
{
	size_t ins_count = _max_active_index;
	for(size_t i = 0; i < ins_count ; i ++) 
	{
		if(_planes_state[i] == 0)
		{       
			continue;
		}       
		world * pPlane = _cur_planes[i];
		if(!pPlane) continue;
		if(pPlane->w_player_count < _player_limit_per_instance)
		{
			world_index = i;
			return pPlane;
		}
	}
	world_index = -1;
	return NULL;
}

world * parallel_world_manager::GetAvailabelWorldRandom(int & world_index)
{
	abase::vector<abase::pair<int, world *> > list;
	list.reserve(_max_active_index);
	size_t ins_count = _max_active_index;
	for(size_t i = 0; i < ins_count ; i ++) 
	{
		if(_planes_state[i] == 0)
		{       
			continue;
		}       
		world * pPlane = _cur_planes[i];
		if(!pPlane) continue;
		if(pPlane->w_player_count < _player_limit_per_instance)
		{
			list.push_back(abase::pair<int, world *>(i, pPlane));
		}
	}
	if(list.size() == 0)
	{
		world_index = -1;
		return NULL;
	}
	int r = abase::Rand(0, list.size()-1);
	world_index = list[r].first;
	return list[r].second;
}

void parallel_world_manager::AutoAllocWorld()
{
	//目前平行世界的key即为world_index,从1开始递增
	spin_autolock keeper(_key_lock);
	int i = 1;
	for( ; i < _planes_capacity; i ++)
	{
		if(!_cur_planes[i]) break;
	}
	if(i < _planes_capacity)
	{
		instance_hash_key hkey;
		hkey.key1 = i;
		hkey.key2 = 0;

		int world_index;
		world * pPlane = AllocWorldWithoutLock(hkey,world_index);
		if(pPlane)
		{
			ASSERT(world_index == i);
		}
		else
		{
			GLog::log(GLOG_WARNING,"自动分配世界失败, world_tag=%d, key/world_index=%d, plane_pool.size=%d, active_plane_count=%d", 
					_world_tag, i, _planes_pool.size(), _active_plane_count);
		}
	}
}


