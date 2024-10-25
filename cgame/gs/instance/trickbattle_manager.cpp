#include <gsp_if.h>
#include "../world.h"
#include "../player_imp.h"
#include "trickbattle_ctrl.h"
#include "trickbattle_manager.h"
#include "../aei_filter.h"

world * trickbattle_world_manager::CreateWorldTemplate()
{
	world * pPlane  = new world;
	pPlane->Init(_world_index);
	pPlane->InitManager(this);
	
	pPlane->SetWorldCtrl(new trickbattle_ctrl());
	return pPlane;
}

world_message_handler * trickbattle_world_manager::CreateMessageHandler()
{
	return new trickbattle_world_message_handler(this);
}

void trickbattle_world_manager::Heartbeat()
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

void trickbattle_world_manager::FinalInit(const char * servername)
{
	_npc_idle_heartbeat = 1;
	if(!city_region::QueryTransportExist(GetWorldTag()))
	{
		__PRINTINFO("ս���ڲ��ܴ��������͵�\n");
		exit(-1);
	}
	
	DATA_TYPE dt;
	CHARIOT_WAR_CONFIG * ess = (CHARIOT_WAR_CONFIG *)GetDataMan().get_data_ptr(TRICKBATTLE_CONFIG_ID, ID_SPACE_CONFIG, dt);
	ASSERT(ess && dt == DT_CHARIOT_WAR_CONFIG);
	for(size_t i=0; i<sizeof(ess->mines)/sizeof(ess->mines[0]); i++)
	{
		if(ess->mines[i].id <= 0 || ess->mines[i].power <= 0) break;
		energy_mine_entry entry;
		entry.mine_tid = ess->mines[i].id;
		entry.energy = ess->mines[i].power;
		_energy_mine_list.push_back(entry);	
	}
}

void trickbattle_world_manager::OnDeliveryConnected()
{
	GMSV::SendTrickBattleServerRegister(GetWorldIndex(),GetWorldTag());
}

int trickbattle_world_manager::OnMineGathered(world * pPlane, int mine_tid, gplayer* pPlayer)
{
	int energy = GetMineEnergy(mine_tid);
	if(energy > 0)
	{
		if(pPlayer->IsBattleOffense() || pPlayer->IsBattleDefence())
		{
			((gplayer_imp *)pPlayer->imp)->TrickBattleIncChariotEnergy(energy);
		}
	}
	return 0;
}

void trickbattle_world_manager::UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size, bool isshielduser, char flag)
{
	//ս�����޷�ֱ�ӵ�¼��
	GMSV::SendLoginRe(cs_index,uid,cs_sid,3,flag);       // login failed
}

void 
trickbattle_world_manager::SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey)
{
	pImp->_filters.AddFilter(new aetb_filter(pImp,FILTER_CHECK_INSTANCE_KEY));
}

void 
trickbattle_world_manager::PlayerAfterSwitch(gplayer_imp * pImp)
{
	trickbattle_switch_data * pData = substance::DynamicCast<trickbattle_switch_data>(pImp->_switch_additional_data);
	if(pData)
	{
		pImp->EnterTrickBattleStep2();
	}
	else
	{
		pImp->ClearSwitchAdditionalData();
	}
}

void trickbattle_world_manager::GetLogoutPos(gplayer_imp * pImp, int & world_tag ,A3DVECTOR & pos)
{
	//����Ӧ���ö�̬��savepoint ��������ʱ��Ҫָ����Щ����
	pImp->GetLastInstanceSourcePos(world_tag,pos);
}

bool trickbattle_world_manager::GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag)
{
	return GetTown(pImp->GetFaction(),pos,tag);
}

void trickbattle_world_manager::RecordTownPos(const A3DVECTOR &pos,int faction)
{
	ASSERT(faction & ( FACTION_BATTLEOFFENSE | FACTION_BATTLEDEFENCE | FACTION_OFFENSE_FRIEND | FACTION_DEFENCE_FRIEND));
	town_entry ent = {faction,pos};
	_town_list.push_back(ent);
}

void trickbattle_world_manager::SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos, int special_mask)
{
	world * pPlane = pPlayer->imp->_plane;
	trickbattle_ctrl* pCtrl = (trickbattle_ctrl *)pPlane->w_ctrl;
	
	//����Ҽ����������ٵ�һ��
	int faction = 0;
	if(pCtrl->_data.attacker_count < pCtrl->_data.defender_count)
	{
		faction = FACTION_OFFENSE_FRIEND | FACTION_BATTLEOFFENSE;
		pPlayer->SetBattleOffense();
	}
	else
	{
		faction = FACTION_BATTLEDEFENCE | FACTION_DEFENCE_FRIEND;
		pPlayer->SetBattleDefence();
	}

	int tag;
	if(GetTown(faction,pPlayer->pos,tag)) return;

	instance_world_manager::SetIncomingPlayerPos(pPlayer,origin_pos,special_mask);	
}

int trickbattle_world_manager::CheckPlayerSwitchRequest(const XID & who,const instance_key * ikey,const A3DVECTOR & pos,int ins_timer)
{
	if(ikey->target.key_level4 == 0)
	{
		return S2C::ERR_CANNOT_ENTER_INSTANCE;
	}

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
		if(pPlane->w_player_count >= _player_limit_per_instance) 
		{
			//��������������
			rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
		}
		else
		{
			trickbattle_ctrl * pCtrl = (trickbattle_ctrl*)pPlane->w_ctrl;

			//��������Ƿ��Ѿ�����
			if(pCtrl->_data.attacker_count >= pCtrl->_data.player_count_limit 
					&& pCtrl->_data.defender_count >= pCtrl->_data.player_count_limit)
			{
				rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
			}

			if(!rst)
			{
				//��������Ƿ��Ѿ������ر�
				if(pCtrl->_data.end_timestamp <= g_timer.get_systime())
				{
					rst = S2C::ERR_BATTLEFIELD_IS_CLOSED;
				}
				else if(pPlane->w_battle_result)
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

	//�����ҵ������� ״̬�����������Ƿ�ƥ��
	mutex_spinunlock(&_key_lock);
	return rst;
}

world * trickbattle_world_manager::GetWorldInSwitch(const instance_hash_key & ikey,int & world_index,int )
{
	spin_autolock keeper(_key_lock);
	world *pPlane = NULL;
	int * pTmp = _key_map.nGet(ikey);
	world_index = -1;
	if(pTmp)
	{
		//�������������� 
		world_index = *pTmp;;
		pPlane = _cur_planes[world_index];
		ASSERT(pPlane);

		//����Ҫ��������Ƿ��������  ���������¼,��ֱ�ӷ���NULL
		//$$$$$$ 
		
		pPlane->w_obsolete = 0;
	}
	if(world_index < 0) return NULL;
	return pPlane;
}

bool trickbattle_world_manager::CreateTrickBattle(const trick_battle_param & param)
{
	//����ȡ�û��ߴ���һ������ 
	spin_autolock keeper(_key_lock);
	//�������ȫ�̱��ּ�����ȷ��״̬��ȷ�����ڴ��ֲ�������������ȫ�̼���Ӧ�ò������̫��ĳ�ͻ 

	//��ʼ��������,��������ķ��䷽ʽҪ������ͬ�Ŷ�(���߿�������NPC����������,�ٸ��ݲ�ͬ������������Ƿ���ĳЩNPC��ʧ��) 
	instance_hash_key hkey;
	hkey.key1 = param.battle_id;
	hkey.key2 = 0;
	int world_index;
	world * pPlane = AllocWorldWithoutLock(hkey,world_index);

	if(pPlane == NULL)
	{
		return false;
	}
	
	//����Ϣ����ʽ֪ͨ����������, �ô�������������ȷ��������߼���ĳ��NPC
	trickbattle_ctrl * pCtrl = dynamic_cast<trickbattle_ctrl*>(pPlane->w_ctrl);
	if(pCtrl == NULL)
	{
		//�����ڲ���ctrl���ǺϷ���
		ASSERT(false);
		return false;
	}

	if(pCtrl->_data.battle_id != 0)
	{
		//������һ��ȫ�¿�����ս��
		return false;
	}
	
	pCtrl->_data.battle_id = param.battle_id;
	pCtrl->_data.attacker_count = 0;
	pCtrl->_data.defender_count = 0; 
	pCtrl->_data.player_count_limit = param.player_count;
	pCtrl->_data.end_timestamp = param.end_timestamp;

	pPlane->w_destroy_timestamp = param.end_timestamp + 300;
	__PRINTF("create battle %d , attacker %d, defender %d\n",param.battle_id);
	__PRINTF("%p world %d ϵͳʱ��%d\n",pPlane,pPlane->w_destroy_timestamp, g_timer.get_systime());

	return true;
}



void 
trickbattle_world_message_handler::PlayerPreEnterServer(gplayer * pPlayer, gplayer_imp * pImp,instance_key & ikey)
{	
	world * pPlane = pImp->_plane;
	trickbattle_ctrl * pCtrl = (trickbattle_ctrl*)(pPlane->w_ctrl);

	if(pPlayer->IsBattleOffense())
	{
		//����
		//����������� ע�����������������player_battle���PlayerLeaveWorld������
		if(!pCtrl->AddAttacker())
		{
			//���ս���ı�־(��������)
			pPlayer->ClrBattleMode();
		}
	}
	else if(pPlayer->IsBattleDefence())
	{	
		//�ط�
		//����������� ע�����������������player_battle���PlayerLeaveWorld������
		if(!pCtrl->AddDefender())
		{
			//���ս���ı�־(��������)
			pPlayer->ClrBattleMode();
		}
	}
	else
	{
		ASSERT(false && "SetIncomingPlayerPos�������Ѿ������˹��ط�");
	}
}

