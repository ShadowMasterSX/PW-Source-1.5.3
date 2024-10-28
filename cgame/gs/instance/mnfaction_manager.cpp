#include <gsp_if.h>
#include "../world.h"
#include "../player_imp.h"
#include "mnfaction_manager.h"
#include "mnfaction_ctrl.h"
#include "../aei_filter.h"

world * mnfaction_world_manager::CreateWorldTemplate()
{
	world * pPlane = new world;
	pPlane->Init(_world_index);
	pPlane->InitManager(this);

	pPlane->SetWorldCtrl(new mnfaction_ctrl());
	return pPlane;
}

world_message_handler * mnfaction_world_manager::CreateMessageHandler()
{
	return new mnfaction_world_message_handler(this);
}

void mnfaction_world_manager::OnDeliveryConnected()
{
	GMSV::SendMNFactionServerRegister(GetWorldIndex(),GetWorldTag());	
}

int mnfaction_world_manager::CreateMNFactionBattle(const mnfaction_battle_param & param)
{
	if(have_creat_battle_domain_ids.find(param.domain_id) != have_creat_battle_domain_ids.end())
	{
		//�ظ�Э��
		return BATTLE_CREATE_ALREADY;
	}
	spin_autolock keeper(_key_lock);
	instance_hash_key hkey;
	hkey.key1 = param.domain_id;
	hkey.key2 = 0;
	int world_index;
	world * pPlane = AllocWorldWithoutLock(hkey,world_index);
	if(pPlane == NULL)
	{
		return BATTLE_CREATE_FAILED;
	}
	
	mnfaction_ctrl * pCtrl = dynamic_cast<mnfaction_ctrl*>(pPlane->w_ctrl);
	if(pCtrl == NULL)
	{
		return BATTLE_CREATE_FAILED;
	}
	
	if(pCtrl->_domain_id != -1)
	{
		return BATTLE_CREATE_FAILED;
	}

	pCtrl->_domain_type               = param.domain_type;
	pCtrl->_domain_id                 = param.domain_id;
	pCtrl->_owner_faction_id          = param.owner_faction_id;
	int randfaction = abase::Rand(0,1);
	//���طֱ����ڵ�ͼ��������,Ϊʹ����������䵽���ˣ����й�����Ӫ�����
	//���һ������ռ��һ����ض�죬���������ص�ս����һֱ�Ƿ��ط������
	//���������������������ڴ˵�ս����һֱ�ڵ�ͼ��һ��,��˽������,���ʱ��¼һ��
	//�����ķ��ذ��ɣ���Ϊ�����ƽ�֣��˵ص�ԭӵ����Ӧ�û�ʤ
	if(randfaction == 1)
	{
		pCtrl->_attacker_faction_id       = param.defender_faction_id;
		pCtrl->_defender_faction_id       = param.attacker_faction_id;
	}
	else
	{
		pCtrl->_attacker_faction_id       = param.attacker_faction_id;
		pCtrl->_defender_faction_id       = param.defender_faction_id;
	}
	pCtrl->_start_timestamp           = g_timer.get_systime();
	pCtrl->_end_timestamp             = param.end_timestamp;
	pCtrl->Init(pPlane);
	
	pPlane->w_destroy_timestamp = param.end_timestamp + 300;
	__PRINTF("create mnfaction_battle %d , attacker %lld, defender %lld\n",param.domain_id, pCtrl->_attacker_faction_id, pCtrl->_defender_faction_id);
	__PRINTF("%p world ����ʱ�� %d ϵͳʱ��%ld\n",pPlane,pPlane->w_destroy_timestamp, pCtrl->_start_timestamp);
	GLog::log(GLOG_INFO,"MNFACTION_LOG create mnfaction_battle domain_id(%d) , owner_faction_id(%lld), attacker_faction_id(%lld), defender_faction_id(%lld), start_timestamp(%d), end_timestamp(%d)\n",param.domain_id, param.owner_faction_id, pCtrl->_attacker_faction_id, pCtrl->_defender_faction_id, pCtrl->_start_timestamp, param.end_timestamp);

	have_creat_battle_domain_ids.insert(param.domain_id);

	GMSV::SetMnDomain(param.domain_id, param.domain_type, param.owner_faction_id, param.attacker_faction_id, param.defender_faction_id);
	return BATTLE_CREATE_SUCCESS;
}

void mnfaction_world_manager::FinalInit(const char * servername)
{
	DATA_TYPE dt;
	MNFACTION_WAR_CONFIG * ess = (MNFACTION_WAR_CONFIG *)world_manager::GetDataMan().get_data_ptr(MNFACTION_CONFIG_ID, ID_SPACE_CONFIG, dt);
	ASSERT(ess && dt == DT_MNFACTION_WAR_CONFIG);
	attacker_boss_tid = ess->attacker_boss_tid;
	defender_boss_tid = ess->defender_boss_tid;
	attacker_small_boss_tid = ess -> attacker_small_boss_tid;
	defender_small_boss_tid = ess -> defender_small_boss_tid;
}

int mnfaction_world_manager::CheckPlayerSwitchRequest(const XID & who,const instance_key * ikey,const A3DVECTOR & pos,int ins_timer)
{
//�����ɸ������͹���
//���ȼ��Key�Ƿ����
	instance_hash_key key;
	TransformInstanceKey(ikey->target,key);
	world *pPlane = NULL;
	int rst = 0;
	mutex_spinlock(&_key_lock);
	int * pTmp = _key_map.nGet(key);
	if(!pTmp)
	{
		mutex_spinunlock(&_key_lock);
		return S2C::ERR_BATTLEFIELD_IS_CLOSED;
	}
	pPlane = _cur_planes[*pTmp];
	if(pPlane)
	{
		if(!(ikey->special_mask & IKSM_GM) && pPlane->w_player_count >= _player_limit_per_instance) 
		{
			//��������������
			rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
		}
		else
		{
			//�������Ƿ����Ҫ��
			mnfaction_ctrl * pCtrl = (mnfaction_ctrl*)pPlane->w_ctrl;

			//��������Ƿ��Ѿ�����
			/*if(pCtrl->_attacker_faction_id == faction_id)
			{
				if(pCtrl->_attend_attacker_player_count >= MAX_ATTACKER_PLAYER_COUNT)
				{
					rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
				}
			}
			else if(pCtrl->_defender_faction_id == faction_id)
			{
				if(pCtrl->_attend_defender_player_count >= MAX_DEFENDER_PLAYER_COUNT)
				{
					rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
				}
			}
			else 
			{
				rst = S2C::ERR_FACTION_IS_NOT_MATCH;
			}*/

			if(!rst)
			{
				//��������Ƿ��Ѿ������ر�
				if(pCtrl->_end_timestamp <= g_timer.get_systime())
				{
					rst = S2C::ERR_BATTLEFIELD_IS_CLOSED;
				}
				if(pPlane->w_battle_result)
				{
					rst = S2C::ERR_BATTLEFIELD_IS_FINISHED;
				}
			}
		}
	}
	else
	{
		rst = S2C::ERR_CANNOT_ENTER_INSTANCE;
	}
	mutex_spinunlock(&_key_lock);
	return rst;
}

void mnfaction_world_manager::SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos, int special_mask)
{
	world * pPlane = pPlayer->imp->_plane;
	mnfaction_ctrl* pCtrl = (mnfaction_ctrl *)pPlane->w_ctrl;

	int64_t faction_id = 0;
	((gplayer_imp *)(pPlayer->imp)) ->GetDBMNFactionInfo(faction_id);
	if(!faction_id) return;
	if(faction_id == pCtrl->_attacker_faction_id)//Ϊ��ʹ�󲿷���ҽ���ս�����ٿ�ս��ս����ʼǰ5���ӵ���ұ���������㣬5���Ӻ��ٴ��͵�����
	{
		if(pCtrl -> GetIsFirstTransmit())
			pPlayer->pos = pCtrl -> _resurrect_pos[0];
		else
			pPlayer->pos = pCtrl -> _attacker_incoming_pos;
	}
	else if(faction_id == pCtrl->_defender_faction_id)
	{
		if(pCtrl -> GetIsFirstTransmit())
			pPlayer->pos = pCtrl -> _resurrect_pos[1];
		else
			pPlayer->pos = pCtrl -> _defender_incoming_pos;
	}
}

void mnfaction_world_manager::GetLogoutPos(gplayer_imp * pImp, int &world_tag, A3DVECTOR & pos)
{
	pImp->GetCarnivalKickoutPos(world_tag, pos);
}

instance_hash_key  mnfaction_world_manager::GetLogoutInstanceKey(gplayer_imp *pImp) const
{
	int groupid = 0;
	world_manager::GetCentralServerBrithPos(pImp->GetSrcZoneId(), groupid);
	return instance_hash_key(groupid, 0);
}

void mnfaction_world_manager::UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size, bool isshielduser, char flag)
{
	//ս�����޷�ֱ�ӵ�¼��
	GMSV::SendLoginRe(cs_index,uid,cs_sid,3,flag);       // login failed
}

void mnfaction_world_manager::SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey)
{
	pImp->_filters.AddFilter(new aemf_filter(pImp,FILTER_CHECK_INSTANCE_KEY,ikey->target.key_level4));
}

void mnfaction_world_manager::PlayerAfterSwitch(gplayer_imp * pImp)
{
	pImp->MnfactionJoinStep2();
}

int mnfaction_world_manager::CanBeGathered(int player_faction, int mine_tid, world *pPlane, const XID &player_xid)
{
	mnfaction_ctrl * pCtrl = (mnfaction_ctrl*)(pPlane->w_ctrl);
	return pCtrl -> CanBeGathered(player_faction, mine_tid, player_xid);
}

int mnfaction_world_manager::OnMineGathered(world * pPlane, int mine_tid, gplayer* pPlayer)
{
	mnfaction_ctrl * pCtrl = (mnfaction_ctrl*)(pPlane->w_ctrl);
	return pCtrl -> OnMineGathered(mine_tid, pPlayer);
}

int mnfaction_world_manager::OnMobDeath(world * pPlane, int faction,  int tid)
{
	mnfaction_ctrl * pCtrl = (mnfaction_ctrl*)(pPlane->w_ctrl);
	if(tid == attacker_boss_tid && (faction & FACTION_BATTLEOFFENSE))
	{
		pCtrl -> OnBossDeath(FACTION_BATTLEOFFENSE, pPlane);	
	}
	if(tid == defender_boss_tid && (faction & FACTION_BATTLEDEFENCE))
	{
		pCtrl -> OnBossDeath(FACTION_BATTLEDEFENCE, pPlane);	
	}
	if(tid == attacker_small_boss_tid && (faction & FACTION_BATTLEOFFENSE))
	{
		pCtrl -> OnSmallBossDeath(FACTION_BATTLEOFFENSE, pPlane);
	}
	if(tid == defender_small_boss_tid && (faction & FACTION_BATTLEDEFENCE))
	{
		pCtrl -> OnSmallBossDeath(FACTION_BATTLEDEFENCE, pPlane);
	}
	return 0;
}

bool mnfaction_world_manager::GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag)
{
	mnfaction_ctrl * pCtrl = (mnfaction_ctrl*)(pImp->_plane->w_ctrl);
	pCtrl -> GetTownPosition(pImp, pos);
	tag = world_manager::GetWorldTag();
	return true;
}

void mnfaction_world_manager::Heartbeat()
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
		//������г�ʱʱ��Ĵ���
		for(size_t i = 0; i < ins_count ; i ++)
		{
			if(_planes_state[i] == 0) continue;	//������
			world * pPlane = _cur_planes[i];
			if(!pPlane) continue;
			if(pPlane->w_obsolete)
			{
				//���ڵȴ��ϳ�״̬
				if(pPlane->w_player_count)
				{
					pPlane->w_obsolete = 0;
				}
				else
				{
					if(pPlane->w_destroy_timestamp <= g_timer.get_systime())
					{
						//û����ұ�����20������Ӧ�ý����world�ع鵽������
						mnfaction_ctrl * pCtrl = dynamic_cast<mnfaction_ctrl*>(pPlane->w_ctrl); 
						if(pCtrl)
						{
							std::set<int>::iterator it = have_creat_battle_domain_ids.find(pCtrl -> _domain_id);
							if(it != have_creat_battle_domain_ids.end())
								have_creat_battle_domain_ids.erase(it);
						}
						FreeWorld(pPlane,i);
					}
				}
			}
			else
			{
				if(!pPlane->w_player_count)
				{
					pPlane->w_obsolete = 1;
				}
			}
			
		}
		_heartbeat_counter = 0;

		//������ȴ�б�Ĵ��� ��Զ������������
		RegroupCoolDownWorld();
	}

	if((++_heartbeat_counter2) > TICK_PER_SEC*HEARTBEAT_CHECK_INTERVAL)
	{
		//�������ص��������㣬��������´�������
		FillWorldPool();

		_heartbeat_counter2 = 0;
	}

	mutex_spinunlock(&_heartbeat_lock);

}
