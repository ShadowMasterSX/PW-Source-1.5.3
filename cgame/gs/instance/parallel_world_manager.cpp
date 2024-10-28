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
		//ÿ10�����һ��
		for(size_t i = 0; i < ins_count ; i ++)
		{
			if(_planes_state[i] == 0) continue;	//������
			world * pPlane = _cur_planes[i];
			if(!pPlane) continue;
			if(pPlane->w_obsolete)
			{
				//���ڵȴ��ϳ�״̬
				if(pPlane->w_player_count || !WorldCapacityHigh())
				{
					pPlane->w_obsolete = 0;
				}
				else
				{
					if((pPlane->w_obsolete += HEARTBEAT_CHECK_INTERVAL) > _idle_time)
					{
						//û����Ҳ���ʱ�䳬ʱ�ˣ����ͷ�����
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

		//������ȴ�б�Ĵ���
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
		//�������ص��������㣬��������´�������
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
	ASSERT(pPlayer->spinlock && "Player����Ϊ����״̬");
	gplayer_imp * pImp = (gplayer_imp *)(pPlayer->imp);
	world * pPlane = pImp->_plane;
	
	//���Ŀ��world�Ƿ�ɽ���
	world * new_pPlane = NULL;
	{
		spin_autolock keeper(_key_lock);
		int * pTmp = _key_map.nGet(hkey);
		if(!pTmp) return S2C::ERR_PARALLEL_WORLD_NOT_EXIST;
		new_pPlane = _cur_planes[*pTmp];
		ASSERT(new_pPlane);
		if(new_pPlane == pPlane)
		{
			__PRINTF("Ŀ��ƽ���������Լ�,�޷��л�\n");
			return S2C::ERR_CANNOT_SWITCH_IN_PARALLEL_WORLD;
		}
		if(new_pPlane->w_player_count >= _player_limit_per_instance)
		{
			__PRINTF("Ŀ��ƽ�������������\n");
			return S2C::ERR_TOO_MANY_PLAYER_IN_PARALLEL_WORLD;
		}
	}
	if(new_pPlane->FindPlayer(pPlayer->ID.id) >= 0)
	{
		ASSERT(false);
		GLog::log(GLOG_ERR, "���(id=%d)��ǰ����world(widx=%d createtime=%d)�У���ͬʱ������world2(widx=%d, createtime=%d)�Ĳ�ѯ����", 
				pPlayer->ID.id, pPlane->w_plane_index, pPlane->w_create_timestamp, new_pPlane->w_plane_index, new_pPlane->w_create_timestamp);
		return S2C::ERR_CANNOT_SWITCH_IN_PARALLEL_WORLD;
	}
	//����Χ��ҷ�����ʧ��Ϣ
	pImp->_runner->disappear();
		
	pImp->PlayerLeaveParallelWorld();
	//��ԭworld�ĸ���ӳ�����ɾ�����
	if(pPlayer->pPiece)
	{
		pPlane->RemovePlayer(pPlayer);
	}
	pPlane->UnmapPlayer(pPlayer->ID.id);
	pPlane->DetachPlayer(pPlayer);
	//��world����ӳ������������
	ASSERT(pPlayer->pPiece == NULL && pPlayer->plane == NULL);
	new_pPlane->AttachPlayer(pPlayer);
	if(!new_pPlane->MapPlayer(pPlayer->ID.id,new_pPlane->GetPlayerIndex(pPlayer)))
	{
		ASSERT(false);	
	}
	SetPlayerWorldIdx(pPlayer->ID.id,new_pPlane->w_plane_index);
	new_pPlane->InsertPlayer(pPlayer);
	//����imp�е�pPlane
	pImp->ResetPlane(new_pPlane);

	pImp->PlayerEnterParallelWorld();
	//���·����������
	pImp->_runner->notify_pos(pPlayer->pos);
	pImp->_runner->begin_transfer();
	pImp->_runner->enter_world();
	pImp->_runner->end_transfer();

	GLog::log(GLOG_INFO,"�û�%d(%d,%d)������ƽ�������л�(%d->%d)",
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
	//��ҽ��볡��ʱδָ�������ָ������������������ͼΪ���ѡ��һ������
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
		//�������������� 
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
	//����ϴε�½ʱ����������ʧ������������������ͼΪ���ѡ��һ������
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
	//Ŀǰƽ�������key��Ϊworld_index,��1��ʼ����
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
			GLog::log(GLOG_WARNING,"�Զ���������ʧ��, world_tag=%d, key/world_index=%d, plane_pool.size=%d, active_plane_count=%d", 
					_world_tag, i, _planes_pool.size(), _active_plane_count);
		}
	}
}


