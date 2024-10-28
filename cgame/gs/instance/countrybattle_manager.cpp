#include <gsp_if.h>
#include "../world.h"
#include "../player_imp.h"
#include "countrybattle_ctrl.h"
#include "countrybattle_manager.h"
#include "../aei_filter.h"

world * countrybattle_world_manager::CreateWorldTemplate()
{
	world * pPlane  = new world;
	pPlane->Init(_world_index);
	pPlane->InitManager(this);
	
	pPlane->SetWorldCtrl(new countrybattle_ctrl());
	return pPlane;
}

world_message_handler * countrybattle_world_manager::CreateMessageHandler()
{
	return new countrybattle_world_message_handler(this);
}

void countrybattle_world_manager::Heartbeat()
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
		//这里进行超时时间的处理
		for(size_t i = 0; i < ins_count ; i ++)
		{
			if(_planes_state[i] == 0) continue;	//空世界
			world * pPlane = _cur_planes[i];
			if(!pPlane) continue;
			if(pPlane->w_obsolete)
			{
				//处于等待废除状态
				if(pPlane->w_player_count)
				{
					pPlane->w_obsolete = 0;
				}
				else
				{
					if(pPlane->w_destroy_timestamp <= g_timer.get_systime())
					{
						//没有玩家保持了20分钟则应该将这个world回归到空闲中
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

		//进行冷却列表的处理 永远都不回收世界
		RegroupCoolDownWorld();
	}

	if((++_heartbeat_counter2) > TICK_PER_SEC*HEARTBEAT_CHECK_INTERVAL)
	{
		//如果世界池的容量不足，则进行重新创建处理
		FillWorldPool();

		_heartbeat_counter2 = 0;
	}

	mutex_spinunlock(&_heartbeat_lock);

}

void countrybattle_world_manager::PreInit(const char * servername)
{
	std::string section = "Instance_";
	section += servername;
	Conf *conf = Conf::GetInstance();
	int battle_type = atoi(conf->find(section,"battle_type").c_str());
	switch(battle_type)
	{
		case BATTLE_TYPE_FLAG:
			_battle_type = BATTLE_TYPE_FLAG;
			__PRINTINFO("国战副本: 夺旗模式\n");
			break;

		case BATTLE_TYPE_TOWER:
			_battle_type = BATTLE_TYPE_TOWER;
			__PRINTINFO("国战副本: 摧毁防御塔模式\n");
			break;
	
		case BATTLE_TYPE_STRONGHOLD:
			_battle_type = BATTLE_TYPE_STRONGHOLD;	
			__PRINTINFO("国战副本: 据点模式\n");
			break;
			
		default:
			_battle_type = BATTLE_TYPE_FLAG;
			__PRINTINFO("国战副本: 模式配置有误，使用默认的夺旗模式\n");
			break;
	}
}

void countrybattle_world_manager::FinalInit(const char * servername)
{
	_npc_idle_heartbeat = 1;
	if(!city_region::QueryTransportExist(GetWorldTag()))
	{
		__PRINTINFO("战场内不能存在区域传送点\n");
		exit(-1);
	}

	DATA_TYPE dt;
	COUNTRY_CONFIG * ess = (COUNTRY_CONFIG *)GetDataMan().get_data_ptr(COUNTRYBATTLE_CONFIG_ID, ID_SPACE_CONFIG, dt);
	ASSERT(ess && dt == DT_COUNTRY_CONFIG);
	
	for(size_t i=0; i<sizeof(ess->flag_controller_id)/sizeof(ess->flag_controller_id[0]); i++)
	{
		if(ess->flag_controller_id[i] <= 0) break;
		_flag_controller_list.push_back(ess->flag_controller_id[i]);
	}
	_flag_mine_tid = ess->flag_mine_id;
	ASSERT(_battle_type != BATTLE_TYPE_FLAG || _flag_controller_list.size() && _flag_mine_tid > 0);
	_attacker_flag_goal.pos = A3DVECTOR(ess->attack_flag_goal[0],ess->attack_flag_goal[1],ess->attack_flag_goal[2]);
	_attacker_flag_goal.squared_radius = ess->attack_flag_goal_radius * ess->attack_flag_goal_radius;
	_defender_flag_goal.pos = A3DVECTOR(ess->defence_flag_goal[0],ess->defence_flag_goal[1],ess->defence_flag_goal[2]);
	_defender_flag_goal.squared_radius = ess->defence_flag_goal_radius * ess->defence_flag_goal_radius;

	for(size_t i=0; i<sizeof(ess->attack_tower)/sizeof(ess->attack_tower[0]); i++)
	{
		if(ess->attack_tower[i].controller_id <= 0) break;
		tower t;
		t.controller_id = ess->attack_tower[i].controller_id;
		t.npc_tid =  ess->attack_tower[i].id;
		t.group = ess->attack_tower[i].group;
		_attacker_tower_list.push_back(t);
		_total_tower_set.insert(t.npc_tid);
	}
	for(size_t i=0; i<sizeof(ess->defence_tower)/sizeof(ess->defence_tower[0]); i++)
	{
		if(ess->defence_tower[i].controller_id <= 0) break;
		tower t;
		t.controller_id = ess->defence_tower[i].controller_id;
		t.npc_tid =  ess->defence_tower[i].id;
		t.group = ess->defence_tower[i].group;
		_defender_tower_list.push_back(t);
		_total_tower_set.insert(t.npc_tid);
	}
	ASSERT(_battle_type != BATTLE_TYPE_TOWER || _attacker_tower_list.size() && _defender_tower_list.size());

	for(size_t i=0; i<sizeof(ess->stronghold)/sizeof(ess->stronghold[0]); i++)
	{
		COUNTRY_CONFIG::StrongHold & shold = ess->stronghold[i];
		if(shold.state[0].controller_id <= 0 || shold.state[0].mine_id <= 0) break;
		stronghold sh;
		ASSERT(sizeof(shold.state)/sizeof(shold.state[0]) == STRONGHOLD_STATE_COUNT);	
		for(size_t s=0; s<STRONGHOLD_STATE_COUNT; s++)
		{
			ASSERT(shold.state[s].controller_id > 0 && shold.state[s].mine_id > 0);
			sh.data[s].controller_id 	= shold.state[s].controller_id;
			sh.data[s].mine_tid 		= shold.state[s].mine_id;
			_total_stronghold_map[sh.data[s].mine_tid] = s;
		}
		if(shold.radius > 1e-3)
		{
			sh.pos = A3DVECTOR(shold.pos[0],shold.pos[1],shold.pos[2]);
			sh.squared_radius = shold.radius * shold.radius;
		}
		else
		{
			sh.pos = A3DVECTOR(0.f, 0.f, 0.f);
			sh.squared_radius = 0.f;
		}
		_stronghold_list.push_back(sh);
	}
	ASSERT(_battle_type != BATTLE_TYPE_STRONGHOLD || _stronghold_list.size());
}

void countrybattle_world_manager::OnDeliveryConnected()
{
	GMSV::SendCountryBattleServerRegister(1, GetWorldIndex(),GetWorldTag(),_battle_type);
	return ;
}

void countrybattle_world_manager::NotifyCountryBattleConfig(GMSV::CBConfig * config)
{
	_capital_list.clear();
	for(size_t i=0; i<config->capital_count; i++)
	{
		GMSV::CBConfig::CountryCapital & capital = config->capital_list[i];
		SetCapital(capital.country_id, A3DVECTOR(capital.posx,capital.posy,capital.posz), capital.worldtag);
	}
}

int countrybattle_world_manager::OnMobDeath(world * pPlane, int faction,  int tid)
{
	if(_battle_type == BATTLE_TYPE_TOWER)
	{
		if(IsTowerNpc(tid))
		{
			if(pPlane->w_ctrl) pPlane->w_ctrl->OnTowerDestroyed(pPlane, faction & FACTION_BATTLEOFFENSE, tid);
		}
	}
	return 0;
}

int countrybattle_world_manager::OnMineGathered(world * pPlane, int mine_tid, gplayer* pPlayer)
{
	if(_battle_type == BATTLE_TYPE_FLAG)
	{
		if(IsFlagMine(mine_tid))
		{
			if(pPlayer->IsBattleOffense() || pPlayer->IsBattleDefence())
			{
				if(pPlane->w_ctrl) pPlane->w_ctrl->PickUpFlag(pPlayer);
			}
		}
	}
	else if(_battle_type == BATTLE_TYPE_STRONGHOLD)
	{
		if(IsStrongholdMine(mine_tid))
		{
			if(pPlayer->IsBattleOffense() || pPlayer->IsBattleDefence())
			{
				if(pPlane->w_ctrl) pPlane->w_ctrl->OccupyStrongHold(mine_tid, pPlayer);
			}		
		}	
	}
	return 0;
}

int countrybattle_world_manager::GenerateFlag()
{
	if(_battle_type != BATTLE_TYPE_FLAG) return 0;
	return _flag_controller_list[ abase::Rand(0, _flag_controller_list.size()-1) ];
}

bool countrybattle_world_manager::IsReachFlagGoal(bool offense, const A3DVECTOR& pos)
{
	if(_battle_type != BATTLE_TYPE_FLAG) return false;
	if(offense)
		return pos.squared_distance(_attacker_flag_goal.pos) <= _attacker_flag_goal.squared_radius;
	else
		return pos.squared_distance(_defender_flag_goal.pos) <= _defender_flag_goal.squared_radius;
}

const countrybattle_world_manager::TOWER_LIST & countrybattle_world_manager::GetTowerList(bool offense)
{
	if(offense) return _attacker_tower_list;
	else return _defender_tower_list;
}

bool countrybattle_world_manager::CanBeGathered(int player_faction, int mine_tid)
{
	if(_battle_type != BATTLE_TYPE_STRONGHOLD) return true;
	abase::hash_map<int, int>::iterator it = _total_stronghold_map.find(mine_tid);
	if(it == _total_stronghold_map.end()) return true;
	int s = it->second;
	
	if(player_faction & FACTION_BATTLEOFFENSE)
	{
		if(s == STRONGHOLD_STATE_NEUTRAL || s == STRONGHOLD_STATE_DEFENDER_HALF || s == STRONGHOLD_STATE_DEFENDER) return true;
	}
	else if(player_faction & FACTION_BATTLEDEFENCE)
	{
		if(s == STRONGHOLD_STATE_NEUTRAL || s == STRONGHOLD_STATE_ATTACKER_HALF || s == STRONGHOLD_STATE_ATTACKER) return true;
	}
	return false;
}

void countrybattle_world_manager::UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size, bool isshielduser, char flag)
{
	//战场是无法直接登录的
	GMSV::SendLoginRe(cs_index,uid,cs_sid,3,flag);       // login failed
}

void 
countrybattle_world_manager::SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey)
{
	pImp->_filters.AddFilter(new aecb_filter(pImp,FILTER_CHECK_INSTANCE_KEY,ikey->target.key_level5));
}

void countrybattle_world_manager::GetLogoutPos(gplayer_imp * pImp, int & world_tag ,A3DVECTOR & pos)
{
	int country_id = pImp->GetCountryId();
	if(country_id)
	{
		//登出点设置为国家首都
		if(GetCapital(country_id, pos, world_tag)) return;
		world_tag = 143;
		pos = A3DVECTOR(0,0,0);
		GLog::log(GLOG_ERR,"首都信息不存在worldtag=%d roleid=%d country=%d", GetWorldTag(), pImp->_parent->ID.id, country_id);
		return;
	}
	pImp->GetCarnivalKickoutPos(world_tag, pos);
}

bool countrybattle_world_manager::GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag)
{
	if(_battle_type == BATTLE_TYPE_STRONGHOLD)
	{
		gplayer * pPlayer = (gplayer *)pImp->_parent;
		if(pPlayer->IsBattleOffense() || pPlayer->IsBattleDefence())
		{
			if(pImp->_plane->w_ctrl->GetStrongholdNearby(pPlayer->IsBattleOffense(), opos, pos, tag)) return true;
		}
	}
	return GetTown(pImp->GetFaction(),pos,tag);
}

void countrybattle_world_manager::RecordTownPos(const A3DVECTOR &pos,int faction)
{
	ASSERT(faction & ( FACTION_BATTLEOFFENSE | FACTION_BATTLEDEFENCE | FACTION_OFFENSE_FRIEND | FACTION_DEFENCE_FRIEND));
	town_entry ent = {faction,pos};
	_town_list.push_back(ent);
}

void countrybattle_world_manager::SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos, int special_mask)
{
	world * pPlane = pPlayer->imp->_plane;
	countrybattle_ctrl* pCtrl = (countrybattle_ctrl *)pPlane->w_ctrl;
	
	int faction = 0;
	if(pPlayer->GetCountryId() == pCtrl->_data.country_defender)
	{
		faction = FACTION_BATTLEDEFENCE | FACTION_DEFENCE_FRIEND;
	}
	else if(pPlayer->GetCountryId() == pCtrl->_data.country_attacker)
	{
		faction = FACTION_OFFENSE_FRIEND | FACTION_BATTLEOFFENSE;
	}

	if(faction)
	{
		int tag;
		if(GetTown(faction,pPlayer->pos,tag)) return;
	}

	instance_world_manager::SetIncomingPlayerPos(pPlayer,origin_pos,special_mask);	
}

int countrybattle_world_manager::CheckPlayerSwitchRequest(const XID & who,const instance_key * ikey,const A3DVECTOR & pos,int ins_timer)
{
	//检查是否去正确的副本 帮派必须正确 而且 必须存在战场ID
	if(ikey->target.key_level4 == 0 || ikey->target.key_level5 == 0)
	{
		return S2C::ERR_CANNOT_ENTER_INSTANCE;
	}
	int country = ikey->target.key_level5;
	
//检查帮派副本传送规则
//首先检查Key是否存在
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
			//检查基础人数上限
			rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
		}
		else
		{
			//检查帮派是否符合要求
			countrybattle_ctrl * pCtrl = (countrybattle_ctrl*)pPlane->w_ctrl;

			//检查人数是否已经满了
			if(pCtrl->_data.country_attacker == country)
			{
				if(pCtrl->_data.attacker_count >= pCtrl->_data.player_count_limit)
				{
					rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
				}
			}
			else if(pCtrl->_data.country_defender == country)
			{
				if(pCtrl->_data.defender_count >= pCtrl->_data.player_count_limit)
				{
					rst = S2C::ERR_TOO_MANY_PLAYER_IN_INSTANCE;
				}
			}
			else 
			{
				rst = S2C::ERR_FACTION_IS_NOT_MATCH;
			}

			if(!rst)
			{
				//检查世界是否已经即将关闭
				if(pCtrl->_data.end_timestamp <= g_timer.get_systime())
				{
					rst = S2C::ERR_BATTLEFIELD_IS_CLOSED;
				}
				else
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

	//检查玩家的人数， 状态和其他数据是否匹配
	mutex_spinunlock(&_key_lock);
	return rst;
}

world * countrybattle_world_manager::GetWorldInSwitch(const instance_hash_key & ikey,int & world_index,int )
{
	spin_autolock keeper(_key_lock);
	world *pPlane = NULL;
	int * pTmp = _key_map.nGet(ikey);
	world_index = -1;
	if(pTmp)
	{
		//存在这样的世界 
		world_index = *pTmp;;
		pPlane = _cur_planes[world_index];
		ASSERT(pPlane);

		//这里要检查世界是否允许登入  若不允许登录,则直接返回NULL
		//$$$$$$ 
		
		pPlane->w_obsolete = 0;
	}
	if(world_index < 0) return NULL;
	return pPlane;
}

bool countrybattle_world_manager::CreateCountryBattle(const country_battle_param & param)
{
	//首先取得或者创建一个世界 
	spin_autolock keeper(_key_lock);
	//这里必须全程保持加锁以确定状态正确，由于此种操作并不常见，全程加锁应该不会带来太大的冲突 

	//开始分配世界,这里世界的分配方式要有所不同才对(或者考虑所有NPC都创建出来,再根据不同的情况来区分是否让某些NPC消失等) 
	instance_hash_key hkey;
	hkey.key1 = param.battle_id;
	hkey.key2 = 0;
	int world_index;
	world * pPlane = AllocWorldWithoutLock(hkey,world_index);

	if(pPlane == NULL)
	{
		return false;
	}
	
	//以消息的形式通知各个生成器, 让此生成器可以正确的清除或者加入某个NPC
	countrybattle_ctrl * pCtrl = dynamic_cast<countrybattle_ctrl*>(pPlane->w_ctrl);
	if(pCtrl == NULL)
	{
		//世界内部的ctrl不是合法的
		ASSERT(false);
		return false;
	}

	if(pCtrl->_data.battle_id != 0)
	{
		//必须是一个全新开启的战场
		return false;
	}
	
	pCtrl->_data.battle_id = param.battle_id;
	pCtrl->_data.country_attacker = param.attacker;
	pCtrl->_data.country_defender = param.defender;
	pCtrl->_data.attacker_count = 0;
	pCtrl->_data.defender_count = 0; 
	pCtrl->_data.player_count_limit = param.player_count;
	pCtrl->_data.end_timestamp = param.end_timestamp;
	pCtrl->_data.attacker_total = param.attacker_total;
	pCtrl->_data.defender_total = param.defender_total;
	pCtrl->_data.max_total = param.max_total;
	pCtrl->Init(_battle_type);

	pPlane->w_destroy_timestamp = param.end_timestamp + 300;
	__PRINTF("create battle %d , attacker %d, defender %d\n",param.battle_id, param.attacker, param.defender);
	__PRINTF("%p world %d 系统时间%d\n",pPlane,pPlane->w_destroy_timestamp, g_timer.get_systime());

	return true;
}

void countrybattle_world_manager::DestroyCountryBattle(int battleid)
{
	instance_hash_key hkey;
	hkey.key1 = battleid;
	hkey.key2 = 0;
	
	mutex_spinlock(&_key_lock);
	int* pTmp = _key_map.nGet(hkey);
	if(pTmp)
	{
		int world_index = *pTmp;;
		world* pPlane = _cur_planes[world_index];
		ASSERT(pPlane);
		mutex_spinunlock(&_key_lock);

		countrybattle_ctrl* pCtrl = dynamic_cast<countrybattle_ctrl*>(pPlane->w_ctrl);
		ASSERT(pCtrl);

		if(pCtrl->_data.battle_id != battleid) return;
		pCtrl->DestroyCountryBattle(pPlane);
		return;
	}
	
	mutex_spinunlock(&_key_lock);
}

instance_hash_key  countrybattle_world_manager::GetLogoutInstanceKey(gplayer_imp *pImp) const
{
	return instance_hash_key(pImp->GetCountryGroup(),0);
}
