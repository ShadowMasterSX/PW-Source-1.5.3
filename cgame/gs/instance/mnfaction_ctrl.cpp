#include "mnfaction_manager.h"
#include "mnfaction_ctrl.h"
#include <gsp_if.h>
#include "../playermnfaction.h"


void mnfaction_ctrl::Init(world * pPlane)
{
	_attend_attacker_player_count = 0;
	_attend_defender_player_count = 0;
	
	DATA_TYPE dt;
	MNFACTION_WAR_CONFIG * ess = (MNFACTION_WAR_CONFIG *)world_manager::GetDataMan().get_data_ptr(MNFACTION_CONFIG_ID, ID_SPACE_CONFIG, dt);
	ASSERT(ess && dt == DT_MNFACTION_WAR_CONFIG);

	_max_resource_point      = ess -> resource_point;
	_attacker_resource_point = ess -> resource_point;
	_defender_resource_point = ess -> resource_point;
	_resource_tower_destroy_reduce_point = ess -> resource_tower_destroy_reduce_point;

	_small_boss_death_reduce_point       = ess -> small_boss_death_reduce_point;
	_attacker_small_boss_controller_id   = ess -> attacker_small_boss_controller_id;
	_defender_small_boss_controller_id   = ess -> defender_small_boss_controller_id;
	_small_boss_appear_time              = ess -> small_boss_appear_time;
	_debuff_appear_time                  = ess -> debuff_appear_time;
	_debuff_init_ratio                   = ess -> debuff_init_ratio / (100.0f);
	_debuff_enhance_ratio_per_minute     = ess -> debuff_enhance_ratio_per_minute / (100.0f);
	
	A3DVECTOR attacker_incoming_pos(ess->domain.attacker_incoming_pos[0], ess->domain.attacker_incoming_pos[1], ess->domain.attacker_incoming_pos[2]);
	_attacker_incoming_pos = attacker_incoming_pos;
	
	A3DVECTOR defender_incoming_pos(ess->domain.defender_incoming_pos[0], ess->domain.defender_incoming_pos[1], ess->domain.defender_incoming_pos[2]);
	_defender_incoming_pos = defender_incoming_pos;
		
	A3DVECTOR attacker_transmit_pos(ess->domain.attacker_transmit_pos[0], ess->domain.attacker_transmit_pos[2], ess->domain.attacker_transmit_pos[2]);
	_attacker_transmit_pos = attacker_transmit_pos;
	
	A3DVECTOR defender_transmit_pos(ess->domain.defender_transmit_pos[0], ess->domain.defender_transmit_pos[1], ess->domain.defender_transmit_pos[2]);
	_defender_transmit_pos = defender_transmit_pos;

	for(int i = 0; i < 2; ++i)
	{
		A3DVECTOR resurrect_pos(ess->domain.resurrect_pos[i].resurrect_pos[0], ess->domain.resurrect_pos[i].resurrect_pos[1], ess->domain.resurrect_pos[i].resurrect_pos[2]);
		_resurrect_pos[i] = resurrect_pos;
	}
	_resurrect_pos_range = ess->domain.resurrect_pos_range;
	
	_resource_point_range = ess->domain.resource_point_range;
	
	A3DVECTOR resource_point_pos0(ess->domain.resourse_point_pos[0].resourse_point_pos[0], 0, ess->domain.resourse_point_pos[0].resourse_point_pos[2]);
	_resource_point_pos[0] = resource_point_pos0;
	
	A3DVECTOR resource_point_pos1(ess->domain.resourse_point_pos[1].resourse_point_pos[0], 0, ess->domain.resourse_point_pos[1].resourse_point_pos[2]);
	_resource_point_pos[1] = resource_point_pos1;
	
	_degree_total = ess->domain.degree_total;
	_degree_per_person_second = ess->domain.degree_per_person_second;
	_gain_resource_point_per_second = ess->domain.gain_resource_point_per_second;
	_gain_resource_point_interval   = ess->domain.gain_resource_point_interval;
	_reduce_resource_point_on_death = ess->domain.reduce_resource_point_on_death;
	
	_cur_degree.push_back(_degree_total / 2);
	_cur_degree.push_back(_degree_total / 2);
	_degree_attacker = (int)(_degree_total * ((1 - NEUTRAL_DEGREE_PERCENT)/2));
	_degree_defender = (int)(_degree_total * (1-(1 - NEUTRAL_DEGREE_PERCENT)/2));

	for(int i = 0; i < MNFACTION_TRANSMIT_POS_NUM; ++i)
	{
		_transmit_point_pos transmit_pos(i, ess->domain.transmit_pos[i].matter_id, ess->domain.transmit_pos[i].transmit_pos, ess->domain.transmit_pos[i].controller_id);
		_transmit_pos_map[ess->domain.transmit_pos[i].matter_id] = transmit_pos;

		pPlane->TriggerSpawn(transmit_pos._controller_id[2]);
		_transmit_index_to_mine_type.push_back(ess->domain.transmit_pos[i].matter_id);
	}//加载传送点信息

	for(int i = 0; i < 4; ++i)
	{
		_tower tower(i, ess->attacker_resource_tower[i].controller_id, ess->attacker_resource_tower[i].guard_controller_id, ess->attacker_resource_tower[i].matter_id, STATE_ATTACKER, FACTION_BATTLEOFFENSE);
		_attacker_resourse_tower[ess->attacker_resource_tower[i].matter_id] = tower;

		pPlane->TriggerSpawn(tower._controller_id[0]);
		pPlane->TriggerSpawn(tower._guard_controller_id);
		_attacker_resourse_tower_mine_types.insert(ess->attacker_resource_tower[i].matter_id);
	}
	
	for(int i = 0; i < 4; ++i)
	{
		_tower tower(i, ess->defender_resource_tower[i].controller_id, ess->defender_resource_tower[i].guard_controller_id, ess->defender_resource_tower[i].matter_id, STATE_DEFENDER, FACTION_BATTLEDEFENCE);
		_defender_resourse_tower[ess->defender_resource_tower[i].matter_id] = tower;

		pPlane->TriggerSpawn(tower._controller_id[0]);
		pPlane->TriggerSpawn(tower._guard_controller_id);
		_defender_resourse_tower_mine_types.insert(ess->defender_resource_tower[i].matter_id);
	}

	for(int i = 0; i < 4; i++)
	{
		_switch_tower switch_tower(i, ess->neutral_tower[i].controller_id, ess->neutral_tower[i].matter_id);
		_neutral_tower[ess->neutral_tower[i].matter_id] = switch_tower;
		
		pPlane->TriggerSpawn(switch_tower._controller_id[2]);
		_neutral_tower_mine_types.insert(ess->neutral_tower[i].matter_id);
	}
}

void mnfaction_ctrl::Reset()
{
	_domain_type     = 0;
	_domain_id       = -1;
	_owner_faction_id     = 0;
	_attacker_faction_id  = 0;
	_defender_faction_id  = 0;
	_end_timestamp        = 0;
	_attend_attacker_player_count  = 0;
	_attend_defender_player_count  = 0;
	_attacker_resourse_tower.clear();
	_defender_resourse_tower.clear();
	_attacker_resourse_tower_mine_types.clear();
	_defender_resourse_tower_mine_types.clear();
	_neutral_tower.clear();
	_neutral_tower_mine_types.clear();
	_transmit_pos_map.clear();
	_transmit_index_to_mine_type.clear();
	_battle_result = 0;
	_is_small_boss_appear = false;
}

void mnfaction_ctrl::OnPlayerDeath(gplayer * pPlayer, const XID & killer, int player_soulpower,const A3DVECTOR& pos)
{
	if(_battle_result) 
		return;
	spin_autolock l(_lock);
	if(pPlayer->IsBattleOffense())
	{
		//interlocked_decrement(&_attacker_resource_point);
		_attacker_resource_point -= _reduce_resource_point_on_death;
		interlocked_increment(&_defender_killed_player_count);
	}
	else
	{
		//interlocked_decrement(&_defender_resource_point);
		_defender_resource_point -= _reduce_resource_point_on_death;
		interlocked_increment(&_attacker_killed_player_count);
	}
}

void mnfaction_ctrl::CheckBattleResult(world * pPlane)
{
	if(_battle_result) 
		return;

	if(_attacker_resource_point <= 0 &&_defender_resource_point > 0)
	{
		_battle_result = BATTLE_RESULT_WINNER_DEFENCE;
		_winner_faction_id = _defender_faction_id;
		BattleEnd(pPlane);
	}
	else if(_defender_resource_point <=0 && _attacker_resource_point > 0)
	{
		_battle_result = BATTLE_RESULT_WINNER_OFFENSE;
		_winner_faction_id = _attacker_faction_id;
		BattleEnd(pPlane);
	}
	else if(_defender_resource_point <=0 && _attacker_resource_point <= 0)
	{
		if(_defender_resource_point < _attacker_resource_point)
		{
			_battle_result = BATTLE_RESULT_WINNER_OFFENSE;
			_winner_faction_id = _attacker_faction_id;
			BattleEnd(pPlane);
		}
		else if(_defender_resource_point > _attacker_resource_point)
		{
			_battle_result = BATTLE_RESULT_WINNER_DEFENCE;
			_winner_faction_id = _defender_faction_id; 
			BattleEnd(pPlane);
		}
		else
			_battle_result = BATTLE_RESULT_WINNER_TIE;
	}
	
	int timestamp = g_timer.get_systime();
	if(timestamp > _end_timestamp)
	{
		if(_attacker_resource_point == _defender_resource_point)
			_battle_result = BATTLE_RESULT_WINNER_TIE;
		else if(_attacker_resource_point < _defender_resource_point)
		{
			_battle_result = BATTLE_RESULT_WINNER_DEFENCE;
			_winner_faction_id = _defender_faction_id;
			BattleEnd(pPlane);
		}
		else
		{
			_battle_result = BATTLE_RESULT_WINNER_OFFENSE;
			_winner_faction_id = _attacker_faction_id;
			BattleEnd(pPlane); 
		}
	}
	if(_battle_result == BATTLE_RESULT_WINNER_TIE)
	{//TIE
		int64_t domain_owner = _owner_faction_id;//GMSV::GetMnDomainOwner(_domain_id);
		if(domain_owner > 0)
		{
			if(domain_owner == _attacker_faction_id)
			{
				 _winner_faction_id = _attacker_faction_id;
				 _battle_result |= BATTLE_RESULT_WINNER_OFFENSE;
			}
			else
			{
				_winner_faction_id = _defender_faction_id; 
				_battle_result |= BATTLE_RESULT_WINNER_DEFENCE; 
			}
		}
		else
		{
			_battle_result |= BATTLE_RESULT_WINNER_RAND;
			int win_index = abase::Rand(0,1);
			if(win_index == 0)
			{
				_winner_faction_id = _attacker_faction_id;
				_battle_result |= BATTLE_RESULT_WINNER_OFFENSE;
			}
			else
			{
				_winner_faction_id = _defender_faction_id;
				_battle_result |= BATTLE_RESULT_WINNER_DEFENCE;
			}
		}
		BattleEnd(pPlane);	
	}
}

bool mnfaction_ctrl::AddPlayerNodeList(cs_user_map &player_node_list, gplayer *pPlayer)
{
	spin_autolock keeper(_player_node_list_lock);
	int cs_index = pPlayer->cs_index;
	std::pair<int,int> val(pPlayer->ID.id,pPlayer->cs_sid);
	if(cs_index >= 0 && val.first >= 0)
	{
		player_node_list[cs_index].push_back(val);
		return true;
	}
	return false;
}

bool mnfaction_ctrl::DelPlayerNodeList(cs_user_map &player_node_list, gplayer * pPlayer)
{
	spin_autolock keeper(_player_node_list_lock);
	int cs_index = pPlayer->cs_index;
	std::pair<int,int> val(pPlayer->ID.id,pPlayer->cs_sid);
	if(cs_index >= 0 && val.first >= 0)
	{
		cs_user_list & list = player_node_list[cs_index];
		int id = pPlayer->ID.id;
		for(size_t i = 0; i < list.size(); i ++)
		{
			if(list[i].first == id)
			{
				list.erase(list.begin() + i);
				i --;
			}
		}
		return true;
	}
	return false;
}

bool mnfaction_ctrl::AddAttacker(gplayer *pPlayer)
{
	if(_attend_attacker_player_count >= MAX_ATTACKER_PLAYER_COUNT) return false;
	int p = interlocked_increment(&_attend_attacker_player_count);
	interlocked_increment(&_recored_attend_attacker_player_count);
	if(p > MAX_ATTACKER_PLAYER_COUNT)
	{
		interlocked_decrement(&_attend_attacker_player_count);
		return false;
	}
	if(!AddPlayerNodeList(_attend_attacker_player_node_list, pPlayer))
		return false;
	return true;
}

bool mnfaction_ctrl::AddDefender(gplayer *pPlayer)
{
	if(_attend_defender_player_count >= MAX_DEFENDER_PLAYER_COUNT) return false;
	int p = interlocked_increment(&_attend_defender_player_count);
	interlocked_increment(&_recored_attend_defender_player_count);
	if(p > MAX_DEFENDER_PLAYER_COUNT)
	{
		interlocked_decrement(&_attend_defender_player_count);
		return false;
	}
	if(!AddPlayerNodeList(_attend_defender_player_node_list, pPlayer))
		return false;
	return true;
}

bool mnfaction_ctrl::DelAttacker(gplayer *pPlayer)
{
	interlocked_decrement(&_attend_attacker_player_count);
	if(!DelPlayerNodeList(_attend_attacker_player_node_list, pPlayer))
		return false;
	DelPlayerPosInfoOnLeave(pPlayer, _attacker_player_pos_info_map);
	return true;
}

bool mnfaction_ctrl::DelDefender(gplayer *pPlayer)
{
	interlocked_decrement(&_attend_defender_player_count);
	if(!DelPlayerNodeList(_attend_defender_player_node_list, pPlayer))
		return false;
	DelPlayerPosInfoOnLeave(pPlayer, _defender_player_pos_info_map);
	return true;
}

int mnfaction_ctrl::PlayerSetBattleDelayStartime()
{
	return _start_timestamp + FIRST_TRANSMIT_PLAYER_INTERVAL;
}

bool mnfaction_ctrl::GetMnFactionInfo(int &attacker_resource_point, int &defender_resource_point, int &attend_attacker_player_count, int &attend_defender_player_count, abase::vector<int> &cur_degree, abase::vector<struct MNFactionStateInfo> &attacker_resouse_tower_state, abase::vector<struct MNFactionStateInfo> &defender_resouse_tower_state, abase::vector<struct MNFactionStateInfo> &switch_tower_state, abase::vector<struct MNFactionStateInfo> &transmit_pos_state, int &attacker_killed_player_count, int &defender_killed_player_count, bool &is_small_boss_appear)
{
	attacker_resource_point = _attacker_resource_point;
	defender_resource_point = _defender_resource_point;
	attend_attacker_player_count = _attend_attacker_player_count;
	attend_defender_player_count = _attend_defender_player_count;
	attacker_killed_player_count = _attacker_killed_player_count;
	defender_killed_player_count = _defender_killed_player_count;
	is_small_boss_appear         = _is_small_boss_appear;
	
	for(abase::vector<int>::iterator it = _cur_degree.begin(); it != _cur_degree.end(); it++)
	{
		cur_degree.push_back(*it);
	}
	for(RESOURSE_TOWER::iterator it = _attacker_resourse_tower.begin(); it != _attacker_resourse_tower.end(); it++)
	{
		attacker_resouse_tower_state.push_back( MNFactionStateInfo(it->second._index, it->second._state, it->second._owner_faction, it->second._time_out));
	}
	for(RESOURSE_TOWER::iterator it = _defender_resourse_tower.begin(); it != _defender_resourse_tower.end(); it++)
	{
		defender_resouse_tower_state.push_back( MNFactionStateInfo(it->second._index, it->second._state, it->second._owner_faction, it->second._time_out));
	}
	for(SWITCH_TOWER::iterator it = _neutral_tower.begin(); it != _neutral_tower.end(); it++)
	{
		switch_tower_state.push_back( MNFactionStateInfo(it->second._index, it->second._state, it->second._owner_faction, it->second._time_out));
	}
	for(TRANSMIT_POS::iterator it = _transmit_pos_map.begin(); it != _transmit_pos_map.end(); it++)
	{
		transmit_pos_state.push_back( MNFactionStateInfo(it->second._index, it->second._state, it->second._owner_faction, it->second._time_out));
	}

	return true;
}

void mnfaction_ctrl::GetDebuffInfo(int &debuff_appear_time, float &debuff_init_ratio, float &debuff_enhance_ratio_per_minute)
{
	debuff_appear_time              = _debuff_appear_time;
	debuff_init_ratio               = _debuff_init_ratio;
	debuff_enhance_ratio_per_minute = _debuff_enhance_ratio_per_minute;
}

int mnfaction_ctrl::CanBeGathered(int player_faction, int mine_tid, const XID &player_xid)
{
	if(_battle_result) 
		return S2C::ERR_MNFACTION_GATHER_FAILED;
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(player_xid.id,index);
	if(!pPlayer)
		return S2C::ERR_MNFACTION_GATHER_FAILED;
	
	RESOURSE_TOWER::iterator it = _attacker_resourse_tower.find(mine_tid);
	if(it != _attacker_resourse_tower.end())
	{
		if(it -> second._matter_type != mine_tid)
			return S2C::ERR_MNFACTION_GATHER_FAILED;
		if(it -> second._gather_faction == player_faction)
			return S2C::ERR_MNFACTION_FACTION_GATHERING;
		if(it -> second._gather_roleid && it -> second._gather_roleid == player_xid.id)
			return S2C::ERR_MNFACTION_FACTION_GATHERING;
		if(it -> second._state == STATE_ATTACKER && pPlayer->IsBattleOffense())
			return S2C::ERR_MNFACTION_BELONG_TO_OWN_FACTION;
		if(it -> second._state == STATE_DEFENDER)
			return S2C::ERR_MNFACTION_HAVE_DESTROYED;
		return 0;
	}
	
	it = _defender_resourse_tower.find(mine_tid);
	if(it != _defender_resourse_tower.end())
	{
		if(it -> second._matter_type != mine_tid)
			return S2C::ERR_MNFACTION_GATHER_FAILED;
		if(it -> second._gather_faction == player_faction)
			return S2C::ERR_MNFACTION_FACTION_GATHERING;
		if(it -> second._gather_roleid && it -> second._gather_roleid == player_xid.id)
			return S2C::ERR_MNFACTION_FACTION_GATHERING;
		if(it -> second._state == STATE_DEFENDER && pPlayer->IsBattleDefence())
			return S2C::ERR_MNFACTION_BELONG_TO_OWN_FACTION;
		if(it -> second._state == STATE_ATTACKER)
			return S2C::ERR_MNFACTION_HAVE_DESTROYED;
		return 0;
	}

	SWITCH_TOWER::iterator it_switch = _neutral_tower.find(mine_tid);
	if(it_switch != _neutral_tower.end())
	{
		if(it_switch -> second._matter_type != mine_tid)
			return S2C::ERR_MNFACTION_GATHER_FAILED;
		if(it_switch -> second._owner_faction == player_faction)
			return S2C::ERR_MNFACTION_BELONG_TO_OWN_FACTION;
		if(it_switch -> second._gather_faction == player_faction)
			return S2C::ERR_MNFACTION_FACTION_GATHERING;
		if(it_switch -> second._gather_roleid && it_switch -> second. _gather_roleid == player_xid.id)
			return S2C::ERR_MNFACTION_FACTION_GATHERING;
		return 0;
	}

	TRANSMIT_POS::iterator it_transmit = _transmit_pos_map.find(mine_tid);
	if(it_transmit != _transmit_pos_map.end())
		if(it_transmit -> second._matter_type != mine_tid)
			return S2C::ERR_MNFACTION_GATHER_FAILED;
		if(it_transmit -> second._owner_faction == player_faction)
			return S2C::ERR_MNFACTION_BELONG_TO_OWN_FACTION;
		if(it_transmit -> second._gather_faction == player_faction)
			return S2C::ERR_MNFACTION_FACTION_GATHERING;
		if(it_transmit -> second._gather_roleid && it_transmit -> second. _gather_roleid == player_xid.id)
			return S2C::ERR_MNFACTION_FACTION_GATHERING;
		return 0;
}

int mnfaction_ctrl::OnMineGathered(int mine_tid, gplayer* pPlayer)
{
	if(_attacker_resourse_tower_mine_types.find(mine_tid) != _attacker_resourse_tower_mine_types.end())
	{
		return GatherResourseTower(_attacker_resourse_tower, mine_tid, pPlayer);
	}
	else if(_defender_resourse_tower_mine_types.find(mine_tid) != _defender_resourse_tower_mine_types.end())
	{
		return GatherResourseTower(_defender_resourse_tower, mine_tid, pPlayer);
	}
	else if(_neutral_tower_mine_types.find(mine_tid) != _neutral_tower_mine_types.end())
	{
		return GatherNeutralTower(_neutral_tower, mine_tid, pPlayer);
	}
	else if(std::find(_transmit_index_to_mine_type.begin(),_transmit_index_to_mine_type.end(), mine_tid) != _transmit_index_to_mine_type.end())
	{
		return GatherTransmitPos(_transmit_pos_map, mine_tid, pPlayer);
	}
	else
	  return 0;
	return 0;
}

int mnfaction_ctrl::GatherResourseTower(RESOURSE_TOWER & resouce_tower_map, int mine_tid, gplayer *pPlayer)
{
	spin_autolock keeper(_lock);
	RESOURSE_TOWER::iterator it = resouce_tower_map.find(mine_tid);
	if(it == resouce_tower_map.end())
	  return 0;
	_tower &resourse_tower = it -> second;
	if(resourse_tower._owner_faction == FACTION_BATTLEOFFENSE)
	{
		if(pPlayer->IsBattleOffense())
		{
			if(resourse_tower._state == STATE_DEFENDER_GATHER)
			{
				resourse_tower._state = STATE_ATTACKER;
				resourse_tower._time_out = -1;
				resourse_tower._gather_roleid = 0;
				resourse_tower._gather_faction = 0;
				pPlayer->plane->ClearSpawn(resourse_tower._controller_id[1]);
				pPlayer->plane->ClearSpawn(resourse_tower._controller_id[2]);
				pPlayer->plane->TriggerSpawn(resourse_tower._controller_id[0]);
				return 1;
			}
			else
				return 0;
		}
		else
		{
			if(resourse_tower._state == STATE_DEFENDER_GATHER || resourse_tower._state == STATE_DEFENDER)
			{
				return 0;
			}
			else
			{
				resourse_tower._state = STATE_DEFENDER_GATHER;
				resourse_tower._time_out = RESOURCE_TOWER_GATHER_TIME;
				resourse_tower._gather_roleid = pPlayer->ID.id;
				resourse_tower._gather_faction   = ((gplayer_imp*)(pPlayer->imp)) -> GetFaction();
				pPlayer->plane->ClearSpawn(resourse_tower._controller_id[0]);
				pPlayer->plane->ClearSpawn(resourse_tower._controller_id[2]);
				pPlayer->plane->TriggerSpawn(resourse_tower._controller_id[1]);
				return 1;
			}
		}
	}
	else
	{
		if(pPlayer->IsBattleDefence())
		{
			if(resourse_tower._state == STATE_ATTACKER_GATHER)
			{
				resourse_tower._state = STATE_DEFENDER;
				resourse_tower._time_out = -1;
				resourse_tower._gather_roleid = 0;
				resourse_tower._gather_faction = 0;
				pPlayer->plane->ClearSpawn(resourse_tower._controller_id[1]);
				pPlayer->plane->ClearSpawn(resourse_tower._controller_id[2]);
				pPlayer->plane->TriggerSpawn(resourse_tower._controller_id[0]);
				return 1;
			}
			else
				return 0;
		}
		else
		{
			if(resourse_tower._state == STATE_ATTACKER_GATHER || resourse_tower._state == STATE_ATTACKER)//正在采集或损毁
			{
				return 0;
			}
			else
			{
				resourse_tower._state = STATE_ATTACKER_GATHER;
				resourse_tower._time_out = RESOURCE_TOWER_GATHER_TIME;
				resourse_tower._gather_roleid = pPlayer->ID.id;
				resourse_tower._gather_faction   = ((gplayer_imp*)(pPlayer->imp)) -> GetFaction();
				pPlayer->plane->ClearSpawn(resourse_tower._controller_id[0]);
				pPlayer->plane->ClearSpawn(resourse_tower._controller_id[2]);
				pPlayer->plane->TriggerSpawn(resourse_tower._controller_id[1]);
				return 1;
			}
		}
	}
	return 0;
}

int mnfaction_ctrl::GatherNeutralTower(SWITCH_TOWER & neutral_tower_map, int mine_tid, gplayer* pPlayer)
{
	spin_autolock keeper(_lock);
	SWITCH_TOWER::iterator it = neutral_tower_map.find(mine_tid);
	if(it == neutral_tower_map.end())
	  return 0;
	_switch_tower &switch_tower = it -> second;
	if(switch_tower._owner_faction == 0)//中立防御塔未被占领
	{
		if(pPlayer->IsBattleOffense())
		{
			if(switch_tower._state == STATE_NEUTRAL || switch_tower._state == STATE_DEFENDER_GATHER)
			{
				switch_tower._state            = STATE_ATTACKER_GATHER;
				switch_tower._gather_roleid    = pPlayer->ID.id;
				switch_tower._time_out         = NEUTUAL_TOWER_GATHER_TIME;
				switch_tower._gather_faction   = ((gplayer_imp*)(pPlayer->imp)) -> GetFaction();
				
				pPlayer->plane->ClearSpawn(switch_tower._controller_id[0]);
				pPlayer->plane->ClearSpawn(switch_tower._controller_id[2]);
				pPlayer->plane->ClearSpawn(switch_tower._controller_id[3]);
				pPlayer->plane->ClearSpawn(switch_tower._controller_id[4]);
				pPlayer->plane->TriggerSpawn(switch_tower._controller_id[1]);
				return 1;
			}
			else if(switch_tower._state == STATE_ATTACKER_GATHER)
				return 0;//攻方正在采集不允许再次采集
		}
		else
		{
			if(switch_tower._state == STATE_NEUTRAL || switch_tower._state == STATE_ATTACKER_GATHER)
			{
				switch_tower._state            = STATE_DEFENDER_GATHER;
				switch_tower._gather_roleid    = pPlayer->ID.id;
				switch_tower._time_out         = NEUTUAL_TOWER_GATHER_TIME;
				switch_tower._gather_faction   = ((gplayer_imp*)(pPlayer->imp)) -> GetFaction();
				
				pPlayer->plane->ClearSpawn(switch_tower._controller_id[0]);
				pPlayer->plane->ClearSpawn(switch_tower._controller_id[1]);
				pPlayer->plane->ClearSpawn(switch_tower._controller_id[2]);
				pPlayer->plane->ClearSpawn(switch_tower._controller_id[4]);
				pPlayer->plane->TriggerSpawn(switch_tower._controller_id[3]);
				return 1;
			}
			else if(switch_tower._state == STATE_DEFENDER_GATHER)
				return 0;//攻方正在采集不允许再次采集
		}
	}
	else if(switch_tower._owner_faction == FACTION_BATTLEOFFENSE)
	{	
		if(pPlayer->IsBattleOffense())
			return 0;
		else
		{
			switch_tower._state            = STATE_DEFENDER_GATHER;
			switch_tower._gather_roleid    = pPlayer->ID.id;
			switch_tower._time_out         = NEUTUAL_TOWER_GATHER_TIME;
			switch_tower._gather_faction   = ((gplayer_imp*)(pPlayer->imp)) -> GetFaction();
			switch_tower._owner_faction    = 0;
			
			pPlayer->plane->ClearSpawn(switch_tower._controller_id[0]);
			pPlayer->plane->ClearSpawn(switch_tower._controller_id[1]);
			pPlayer->plane->ClearSpawn(switch_tower._controller_id[2]);
			pPlayer->plane->ClearSpawn(switch_tower._controller_id[4]);
			pPlayer->plane->TriggerSpawn(switch_tower._controller_id[3]);
			return 1;
		}
	}
	else
	{
		if(pPlayer->IsBattleOffense())
		{
			switch_tower._state            = STATE_ATTACKER_GATHER;
			switch_tower._gather_roleid    = pPlayer->ID.id;
			switch_tower._time_out         = NEUTUAL_TOWER_GATHER_TIME;
			switch_tower._gather_faction   = ((gplayer_imp*)(pPlayer->imp)) -> GetFaction();
			switch_tower._owner_faction    = 0;
			
			pPlayer->plane->ClearSpawn(switch_tower._controller_id[0]);
			pPlayer->plane->ClearSpawn(switch_tower._controller_id[2]);
			pPlayer->plane->ClearSpawn(switch_tower._controller_id[3]);
			pPlayer->plane->ClearSpawn(switch_tower._controller_id[4]);
			pPlayer->plane->TriggerSpawn(switch_tower._controller_id[1]);
			return 1;
		}
		else
			return 0;
	}
	return 0;
}

int mnfaction_ctrl::GatherTransmitPos(TRANSMIT_POS & transmit_pos_map, int mine_tid, gplayer* pPlayer)
{
	spin_autolock keeper(_lock);
	TRANSMIT_POS::iterator it = _transmit_pos_map.find(mine_tid);
	if(it == _transmit_pos_map.end())
	  return 0;
	_transmit_point_pos &transmit_pos = it -> second;
	if(transmit_pos._owner_faction == 0)
	{
		if(pPlayer->IsBattleOffense())
		{
			if(transmit_pos._state == STATE_NEUTRAL || transmit_pos._state == STATE_DEFENDER_GATHER)
			{
				transmit_pos._state            = STATE_ATTACKER_GATHER;
				transmit_pos._gather_roleid    = pPlayer->ID.id;
				transmit_pos._time_out         = TRANSMIT_POS_GATHER_TIME;
				transmit_pos._gather_faction   = ((gplayer_imp*)(pPlayer->imp)) -> GetFaction();

				pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_ATTACKER]);
				pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_NEUTRAL]);
				pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_DEFENDER_GATHER]);
				pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_DEFENDER]);
				pPlayer->plane->TriggerSpawn(transmit_pos._controller_id[STATE_ATTACKER_GATHER]);
				return 1;
			}
			else if(transmit_pos._state == STATE_ATTACKER_GATHER)
				return 0;//攻方正在采集不允许再次采集
		}
		else
		{
			if(transmit_pos._state == STATE_NEUTRAL || transmit_pos._state == STATE_ATTACKER_GATHER)
			{
				transmit_pos._state            = STATE_DEFENDER_GATHER;
				transmit_pos._gather_roleid    = pPlayer->ID.id;
				transmit_pos._time_out         = TRANSMIT_POS_GATHER_TIME;
				transmit_pos._gather_faction   = ((gplayer_imp*)(pPlayer->imp)) -> GetFaction();
				
				pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_ATTACKER]);
				pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_ATTACKER_GATHER]);
				pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_NEUTRAL]);
				pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_DEFENDER]);
				pPlayer->plane->TriggerSpawn(transmit_pos._controller_id[STATE_DEFENDER_GATHER]);
				return 1;
			}
			else if(transmit_pos._state == STATE_DEFENDER_GATHER)
				return 0;//攻方正在采集不允许再次采集
		}
	}
	else if(transmit_pos._owner_faction == FACTION_BATTLEOFFENSE)
	{	
		if(pPlayer->IsBattleOffense())
			return 0;
		else
		{
			transmit_pos._state            = STATE_DEFENDER_GATHER;
			transmit_pos._gather_roleid    = pPlayer->ID.id;
			transmit_pos._time_out         = TRANSMIT_POS_GATHER_TIME;
			transmit_pos._gather_faction   = ((gplayer_imp*)(pPlayer->imp)) -> GetFaction();
			transmit_pos._owner_faction    = 0;
			
			pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_ATTACKER]);
			pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_ATTACKER_GATHER]);
			pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_NEUTRAL]);
			pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_DEFENDER]);
			pPlayer->plane->TriggerSpawn(transmit_pos._controller_id[STATE_DEFENDER_GATHER]);
			return 1;
		}
	}
	else
	{
		if(pPlayer->IsBattleOffense())
		{
			transmit_pos._state            = STATE_ATTACKER_GATHER;
			transmit_pos._gather_roleid    = pPlayer->ID.id;
			transmit_pos._time_out         = TRANSMIT_POS_GATHER_TIME;
			transmit_pos._gather_faction   = ((gplayer_imp*)(pPlayer->imp)) -> GetFaction();
			transmit_pos._owner_faction    = 0;
			
			pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_ATTACKER]);
			pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_NEUTRAL]);
			pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_DEFENDER_GATHER]);
			pPlayer->plane->ClearSpawn(transmit_pos._controller_id[STATE_DEFENDER]);
			pPlayer->plane->TriggerSpawn(transmit_pos._controller_id[STATE_ATTACKER_GATHER]);
			return 1;
		}
		else
			return 0;
	}
	return 0;
}

void mnfaction_ctrl::CheckResourceTowerState(RESOURSE_TOWER & resourse_tower_map, world * pPlane)
{
	RESOURSE_TOWER::iterator it = resourse_tower_map.begin();
	for(; it != resourse_tower_map.end(); ++it)
	{
		_tower &resourse_tower = it->second;
		if(resourse_tower._state == STATE_ATTACKER || resourse_tower._state == STATE_DEFENDER)
			continue;
		else if(resourse_tower._state == STATE_ATTACKER_GATHER)
		{
			ASSERT(resourse_tower._gather_faction == FACTION_BATTLEOFFENSE && resourse_tower._owner_faction == FACTION_BATTLEDEFENCE);
			--(resourse_tower._time_out);
			if(resourse_tower._time_out == 0)
			{
				resourse_tower._state            = STATE_ATTACKER;//损毁状态
				resourse_tower._gather_roleid    = 0;
				resourse_tower._gather_faction   = 0;
				pPlane->ClearSpawn(resourse_tower._controller_id[0]);
				pPlane->ClearSpawn(resourse_tower._controller_id[1]);
				pPlane->TriggerSpawn(resourse_tower._controller_id[2]);
				ResourceTowerDestroy(FACTION_BATTLEDEFENCE, pPlane);
			}
		}
		else if(resourse_tower._state == STATE_DEFENDER_GATHER)
		{
			ASSERT(resourse_tower._gather_faction == FACTION_BATTLEDEFENCE && resourse_tower._owner_faction == FACTION_BATTLEOFFENSE);
			--(resourse_tower._time_out);
			if(resourse_tower._time_out == 0)
			{
				resourse_tower._state            = STATE_DEFENDER;//损毁
				resourse_tower._gather_roleid    = 0;
				resourse_tower._gather_faction   = 0;
				pPlane->ClearSpawn(resourse_tower._controller_id[0]);
				pPlane->ClearSpawn(resourse_tower._controller_id[1]);
				pPlane->TriggerSpawn(resourse_tower._controller_id[2]);
				ResourceTowerDestroy(FACTION_BATTLEOFFENSE, pPlane);
			}
		}
	}
}

void mnfaction_ctrl::CheckSwitchTowerState(SWITCH_TOWER & neutral_tower_map, world * pPlane)
{
	SWITCH_TOWER::iterator it = neutral_tower_map.begin();
	for(; it != neutral_tower_map.end(); ++it)
	{
		_switch_tower &switch_tower = it -> second;
		if(switch_tower._state == STATE_ATTACKER || switch_tower._state == STATE_DEFENDER || switch_tower._state == STATE_NEUTRAL)
			continue;
		
		else if(switch_tower._state == STATE_ATTACKER_GATHER)
		{
			ASSERT(switch_tower._gather_faction == FACTION_BATTLEOFFENSE);
			--(switch_tower._time_out);
			if(switch_tower._time_out == 0)
			{
				switch_tower._state            = STATE_ATTACKER;
				switch_tower._gather_roleid    = 0;
				switch_tower._gather_faction   = 0;
				switch_tower._owner_faction    = FACTION_BATTLEOFFENSE;
				pPlane->ClearSpawn(switch_tower._controller_id[1]);
				pPlane->ClearSpawn(switch_tower._controller_id[2]);
				pPlane->ClearSpawn(switch_tower._controller_id[3]);
				pPlane->ClearSpawn(switch_tower._controller_id[4]);
				pPlane->TriggerSpawn(switch_tower._controller_id[0]);
			}
		}
		else if(switch_tower._state == STATE_DEFENDER_GATHER)
		{
			ASSERT(switch_tower._gather_faction == FACTION_BATTLEDEFENCE);
			--(switch_tower._time_out);
			if(switch_tower._time_out == 0)
			{
				switch_tower._state            = STATE_DEFENDER;
				switch_tower._gather_roleid    = 0;
				switch_tower._gather_faction   = 0;
				switch_tower._owner_faction    = FACTION_BATTLEDEFENCE;
				pPlane->ClearSpawn(switch_tower._controller_id[0]);
				pPlane->ClearSpawn(switch_tower._controller_id[1]);
				pPlane->ClearSpawn(switch_tower._controller_id[2]);
				pPlane->ClearSpawn(switch_tower._controller_id[3]);
				pPlane->TriggerSpawn(switch_tower._controller_id[4]);
			}
		}
	}
}

void mnfaction_ctrl::CheckTransmitState(TRANSMIT_POS & transmit_pos_map, world * pPlane)
{
	TRANSMIT_POS::iterator it = transmit_pos_map.begin();
	for(; it != transmit_pos_map.end(); ++it)
	{
		_transmit_point_pos &transmit_pos = it -> second;
		if(transmit_pos._state == STATE_ATTACKER || transmit_pos._state == STATE_DEFENDER || transmit_pos._state == STATE_NEUTRAL)
			continue;
		
		else if(transmit_pos._state == STATE_ATTACKER_GATHER)
		{
			ASSERT(transmit_pos._gather_faction == FACTION_BATTLEOFFENSE);
			--(transmit_pos._time_out);
			if(transmit_pos._time_out == 0)
			{
				transmit_pos._state            = STATE_ATTACKER;
				transmit_pos._gather_roleid    = 0;
				transmit_pos._gather_faction   = 0;
				transmit_pos._owner_faction    = FACTION_BATTLEOFFENSE;
				pPlane->ClearSpawn(transmit_pos._controller_id[1]);
				pPlane->ClearSpawn(transmit_pos._controller_id[2]);
				pPlane->ClearSpawn(transmit_pos._controller_id[3]);
				pPlane->ClearSpawn(transmit_pos._controller_id[4]);
				pPlane->TriggerSpawn(transmit_pos._controller_id[0]);
			}
		}
		else if(transmit_pos._state == STATE_DEFENDER_GATHER)
		{
			ASSERT(transmit_pos._gather_faction == FACTION_BATTLEDEFENCE);
			--(transmit_pos._time_out);
			if(transmit_pos._time_out == 0)
			{
				transmit_pos._state            = STATE_DEFENDER;
				transmit_pos._gather_roleid    = 0;
				transmit_pos._gather_faction   = 0;
				transmit_pos._owner_faction    = FACTION_BATTLEDEFENCE;
				pPlane->ClearSpawn(transmit_pos._controller_id[0]);
				pPlane->ClearSpawn(transmit_pos._controller_id[1]);
				pPlane->ClearSpawn(transmit_pos._controller_id[2]);
				pPlane->ClearSpawn(transmit_pos._controller_id[3]);
				pPlane->TriggerSpawn(transmit_pos._controller_id[4]);
			}
		}
	}
}
void mnfaction_ctrl::CheckResourcePointState(world *pPlane)
{
	for(int i = 0; i < 2; ++i)
	{
		int attacker_count = 0;
		int defender_count = 0;
		visible_collector worker(pPlane, attacker_count, defender_count, _resource_point_range);
		pPlane->ForEachSlice(_resource_point_pos[i], _resource_point_range, worker); 
		int move_degree = (defender_count - attacker_count) * _degree_per_person_second;
		_cur_degree[i] += move_degree;
		if(_cur_degree[i] < 0)
			_cur_degree[i] = 0;
		if(_cur_degree[i] > _degree_total)
			_cur_degree[i] = _degree_total;
	}
}

void mnfaction_ctrl::CalResoursePoint()
{
	for(int i = 0; i < 2; ++i)
	{
		if(_cur_degree[i] > _degree_defender)
		{
			_defender_resource_point += _gain_resource_point_per_second;
			if(_defender_resource_point > _max_resource_point)
				_defender_resource_point = _max_resource_point;
		}
		else if(_cur_degree[i] < _degree_attacker)
		{
			_attacker_resource_point += _gain_resource_point_per_second;
			if(_attacker_resource_point > _max_resource_point)
				_attacker_resource_point = _max_resource_point;
		}
	}
}

int mnfaction_ctrl::OnBossDeath(int faction, world * pPlane)
{
	if(_battle_result) 
		return 0;
	spin_autolock l(_lock);
	if(faction == FACTION_BATTLEOFFENSE)
	{	
		_battle_result = BATTLE_RESULT_WINNER_DEFENCE;
		_winner_faction_id = _defender_faction_id;
		BattleEnd(pPlane);
	}
	else
	{
		_battle_result = BATTLE_RESULT_WINNER_OFFENSE;
		_winner_faction_id = _attacker_faction_id;
		BattleEnd(pPlane);
	}
	return 0;
}

int mnfaction_ctrl::OnSmallBossDeath(int faction, world * pPlane)
{
	if(_battle_result) 
		return 0;
	spin_autolock l(_lock);
	if(faction == FACTION_BATTLEOFFENSE)
	{
		_attacker_resource_point -= _small_boss_death_reduce_point;
		_is_small_boss_attacker_death = true;
	}
	else
	{
		_defender_resource_point -= _small_boss_death_reduce_point;
		_is_small_boss_defender_death = true;
	}
	return 0;
}

int mnfaction_ctrl::PlayerTransmitInMNFaction(gplayer_imp * pImp, int index, A3DVECTOR &pos)
{
	gplayer *pPlayer = (gplayer*)(pImp->_parent);
	int transmit_pos_index = _transmit_index_to_mine_type[index];
	_transmit_point_pos &transmit_pos = _transmit_pos_map[transmit_pos_index];
	if(pPlayer->IsBattleOffense())
	{
		if(transmit_pos._owner_faction != FACTION_BATTLEOFFENSE)
			return S2C::ERR_MNFACTION_TRANSMIT_POS_FACTION;
	}
	else
	{
		if(transmit_pos._owner_faction != FACTION_BATTLEDEFENCE)
			return S2C::ERR_MNFACTION_TRANSMIT_POS_FACTION;
	}
	pos = transmit_pos._transmit_pos;
	return 0;
}

void mnfaction_ctrl::BattleEnd(world * pPlane)
{
	pPlane->w_destroy_timestamp = g_timer.get_systime() + 60;
	pPlane->w_battle_result = _battle_result; 
	_battle_end_timer = 5;

	RecordLog();
	GMSV::SetMnDomain(_domain_id, _domain_type, 0, 0, 0);
}

void mnfaction_ctrl::RecordLog()
{
	int end_time = g_timer.get_systime();
	struct tm tm_start;
	struct tm tm_end; 
	localtime_r((time_t *)&_start_timestamp, &tm_start);
	localtime_r((time_t *)&end_time, &tm_end);

	std::ostringstream log;
	log << "MNFACTION_LOG BATTLE_END domain_id(" << _domain_id << "), ";
	log << "domain_type(" << (int)_domain_type << "), ";
	log << "owner_faction_id(" << _owner_faction_id << "), ";
	log << "winner_faction_id(" << _winner_faction_id << "), ";
	log << "attacker_faction_id(" << _attacker_faction_id << "), ";
	log << "defender_faction_id(" << _defender_faction_id << "), ";
	log << "start_timestamp(" << _start_timestamp << "), ";
	log << "end_timestamp(" << _end_timestamp << "), ";
	log << "start(" << (tm_start.tm_year + 1900) << "," << tm_start.tm_mon + 1 << "," << tm_start.tm_mday << "," << tm_start.tm_hour<< "," << tm_start.tm_min << ", " << tm_start.tm_sec << "),"; 
	log << "end(" << (tm_end.tm_year + 1900) << "," << tm_end.tm_mon + 1 << "," << tm_end.tm_mday << "," << tm_end.tm_hour<< "," << tm_end.tm_min << "," << tm_end.tm_sec << "),";
	log << "attacker_point(" << _attacker_resource_point << ")," ;
	log << "defender_point(" << _defender_resource_point << ")," ;
	for(RESOURSE_TOWER::iterator it = _attacker_resourse_tower.begin(); it != _attacker_resourse_tower.end(); ++it)
	{
		log << "attacker_tower_state("<< it->second._index << ":"  << it->second._state <<"),";
	}
	for(RESOURSE_TOWER::iterator it = _defender_resourse_tower.begin(); it != _defender_resourse_tower.end(); ++it)
	{
		log << "defender_tower_state("<< it->second._index << ":"  << it->second._state <<"),";
	}
	for(SWITCH_TOWER::iterator it = _neutral_tower.begin(); it != _neutral_tower.end(); ++it)
	{
		log << "neutral_tower_state("<< it->second._index << ":"  << it->second._state <<"),";
	}
	log << "ever_enter_attacker_player_count(" << _recored_attend_attacker_player_count << "),";
	log << "ever_enter_defender_player_count(" << _recored_attend_defender_player_count << "),";

	log << "attacker_player_count(" << _attend_attacker_player_count << "),";
	log << "defender_player_count(" << _attend_defender_player_count << "),";

	log << "attacker_die_num(" << _defender_killed_player_count << "),";
	log << "defender_die_num(" << _attacker_killed_player_count << "),";

	log << "attacker_small_boss_state(";
	if(!_is_small_boss_appear)
	{
		log << "no appear),";
	}
	else
	{
		if(!_is_small_boss_attacker_death)
		{
			log << "living),";
		}
		else
		{
			log << "death),";
		}
	}
	
	log << "defender_small_boss_state(";
	if(!_is_small_boss_appear)
	{
		log << "no appear),";
	}
	else
	{
		if(!_is_small_boss_defender_death)
		{
			log << "living),";
		}
		else
		{
			log << "death),";
		}
	}
	GLog::log(GLOG_INFO, log.str().c_str());
}

void mnfaction_ctrl::SendDSBattleEnd()
{
	GMSV::SendMnfactionBattleEnd(_domain_id, _winner_faction_id);
}

void mnfaction_ctrl::ResourceTowerDestroy(int faction, world *pPlane)//透明怪根据全局变量减少将军的血量 
{
	int num = 0;
	if(faction & FACTION_BATTLEOFFENSE)
	{
		for(RESOURSE_TOWER::iterator it = _attacker_resourse_tower.begin(); it != _attacker_resourse_tower.end(); ++it)
		{
			if(it -> second._state == STATE_DEFENDER)
			{
				++num;
			}
		}
		pPlane -> SetCommonValue(COMMON_VALUE_ID_MNFACTION_ATTACKER_DESTROY_RESOURSE_TOWER_NUM, num); 
		_attacker_resource_point -= _resource_tower_destroy_reduce_point;
	}
	else
	{
		for(RESOURSE_TOWER::iterator it = _defender_resourse_tower.begin(); it != _defender_resourse_tower.end(); ++it)
		{
			if(it -> second._state == STATE_ATTACKER)
			{
				++num;
			}
		}
		pPlane -> SetCommonValue(COMMON_VALUE_ID_MNFACTION_DEFENDER_DESTROY_RESOURSE_TOWER_NUM, num); 
		_defender_resource_point -= _resource_tower_destroy_reduce_point;
	}
}

void mnfaction_ctrl::TransmitResurrectPosPlayer(world * pPlane)//每30S把复活点(小黑屋)中的玩家传送回自己方的基地
{
	A3DVECTOR pos(0,0,0);
	MSG msg;
	BuildMessage(msg,GM_MSG_LONGJUMP,XID(GM_TYPE_PLAYER,0),XID(0,0),pos,world_manager::GetWorldTag(),&_attacker_incoming_pos,sizeof(_attacker_incoming_pos));
	pPlane -> BroadcastLocalSphereMessage(msg, _resurrect_pos[0], _resurrect_pos_range);//传送攻方复活点玩家
	
	BuildMessage(msg,GM_MSG_LONGJUMP,XID(GM_TYPE_PLAYER,0),XID(0,0),pos,world_manager::GetWorldTag(),&_defender_incoming_pos,sizeof(_defender_incoming_pos));
	pPlane -> BroadcastLocalSphereMessage(msg, _resurrect_pos[1], _resurrect_pos_range);//传送守方复活点玩家
}

void mnfaction_ctrl::GetTownPosition(gplayer_imp *pImp, A3DVECTOR &resurrect_pos)
{
	if(((gplayer*)(pImp->_parent))->IsBattleOffense())
	{
		resurrect_pos = _resurrect_pos[0]; 
	}
	else
	{
		resurrect_pos = _resurrect_pos[1]; 
	}
}

void mnfaction_ctrl::PlayerPosInfoSync(int roleid, int faction, A3DVECTOR &pos)
{
	spin_autolock l(_player_node_list_lock);
	if(faction == FACTION_BATTLEOFFENSE)
	{
		if(_attacker_player_pos_info_map.find(roleid) != _attacker_player_pos_info_map.end())
		{
			_attacker_player_pos_info_map[roleid] = mnfaction_player_pos_info(roleid, pos); 
			__PRINTF("MNFACTION sync player pos info, roleid %d ,  pos %f,%f,%f \n",roleid, pos.x, pos.y, pos.z);
		}
		else
		{
			_attacker_player_pos_info_map[roleid]._player_pos = pos;	
			__PRINTF("MNFACTION sync player pos info, roleid %d ,  pos %f,%f,%f \n",roleid, pos.x, pos.y, pos.z);
		}
	}
	else
	{
		if(_defender_player_pos_info_map.find(roleid) != _defender_player_pos_info_map.end())
		{
			_defender_player_pos_info_map[roleid] = mnfaction_player_pos_info(roleid, pos); 
			__PRINTF("MNFACTION sync player pos info, roleid %d ,  pos %f,%f,%f \n",roleid, pos.x, pos.y, pos.z);
		}
		else
		{
			_defender_player_pos_info_map[roleid]._player_pos = pos;	
			__PRINTF("MNFACTION sync player pos info, roleid %d ,  pos %f,%f,%f \n",roleid, pos.x, pos.y, pos.z);
		}
	}
}

void mnfaction_ctrl::DelPlayerPosInfoOnLeave(gplayer *pPlayer, player_pos_info_map & pos_info_map)
{
	spin_autolock l(_player_node_list_lock);
	player_pos_info_map::iterator it = pos_info_map.find(pPlayer->ID.id);
	if(it != pos_info_map.end())
		pos_info_map.erase(it);
}

void mnfaction_ctrl::SendClientPlayerPosInfo()
{
	spin_autolock keeper(_player_node_list_lock);
	_tbuf.clear();
	using namespace S2C;
	CMD::Make<CMD::mnfaction_player_pos_info>::From(_tbuf, _attacker_player_pos_info_map.size());
	for(player_pos_info_map::iterator it = _attacker_player_pos_info_map.begin(); it != _attacker_player_pos_info_map.end(); ++it)
	{
		mnfaction_player_pos_info &pos_info = it -> second;
		CMD::Make<CMD::mnfaction_player_pos_info>::Add(_tbuf, pos_info. _roleid, pos_info._player_pos.x, pos_info._player_pos.y, pos_info._player_pos.z);
		__PRINTF("MNFACTION send player pos info, roleid %d ,  pos %f,%f,%f \n",pos_info._roleid, pos_info._player_pos.x, pos_info._player_pos.y, pos_info._player_pos.z);
	}
	
	multi_send_ls_msg(_attend_attacker_player_node_list, _tbuf.data(), _tbuf.size(), 0);

	_tbuf.clear();
	using namespace S2C;
	CMD::Make<CMD::mnfaction_player_pos_info>::From(_tbuf, _defender_player_pos_info_map.size());
	for(player_pos_info_map::iterator it = _defender_player_pos_info_map.begin(); it != _defender_player_pos_info_map.end(); ++it)
	{
		mnfaction_player_pos_info &pos_info = it -> second;
		CMD::Make<CMD::mnfaction_player_pos_info>::Add(_tbuf, pos_info._roleid, pos_info._player_pos.x, pos_info._player_pos.y, pos_info._player_pos.z);
		__PRINTF("MNFACTION send player pos info, roleid %d ,  pos %f,%f,%f \n",pos_info._roleid, pos_info._player_pos.x, pos_info._player_pos.y, pos_info._player_pos.z);
	}

	multi_send_ls_msg(_attend_defender_player_node_list, _tbuf.data(), _tbuf.size(), 0);
}

void mnfaction_ctrl::BattleFactionSay(int faction, const void * buf, size_t size, char emote_id, const void * aux_data, size_t dsize, int self_id, int self_level)
{
	spin_autolock keeper(_player_node_list_lock);
	
	if(faction == FACTION_BATTLEOFFENSE)
	{
		multi_send_chat_msg(_attend_attacker_player_node_list,buf,size,GMSV::CHAT_CHANNEL_BATTLE,emote_id,aux_data,dsize,self_id,self_level);	
	}
	else if(faction == FACTION_BATTLEDEFENCE)
	{
		multi_send_chat_msg(_attend_defender_player_node_list,buf,size,GMSV::CHAT_CHANNEL_BATTLE,emote_id,aux_data,dsize,self_id,self_level);	
	}
}

void mnfaction_ctrl::Tick(world * pPlane)
{
	spin_autolock l(_lock);
	if(++ _tick_counter < 20) return;
	_tick_counter = 0;

	if(_battle_end_timer > 0) 
	{
		if(-- _battle_end_timer == 0)
			SendDSBattleEnd();
	}
	else
	{
		CheckResourceTowerState(_attacker_resourse_tower, pPlane);
		CheckResourceTowerState(_defender_resourse_tower, pPlane);
		CheckSwitchTowerState(_neutral_tower, pPlane);
		CheckTransmitState(_transmit_pos_map, pPlane);
		CheckResourcePointState(pPlane);

		++_second_resource_point;
		if(_second_resource_point >= _gain_resource_point_interval)
		{
			_second_resource_point = 0;
			CalResoursePoint();
		}

		CheckBattleResult(pPlane);

		if(++_sync_pos_tick > SEND_ALL_POS_INFO_TIME)
		{
			_sync_pos_tick = 0;
			SendClientPlayerPosInfo();
		}

		int cur_time = g_timer.get_systime();
		if(!_is_small_boss_appear && _end_timestamp > cur_time && _end_timestamp - cur_time < _small_boss_appear_time)
		{
			pPlane->TriggerSpawn(_attacker_small_boss_controller_id);
			pPlane->TriggerSpawn(_defender_small_boss_controller_id);
			_is_small_boss_appear = true;
		}
		if(++_second_resurrect >= _resurrect_interval)
		{
			if(_is_first_transmit_player)
			{
				_resurrect_interval = RESURRECT_POS_INTERVAL;
				_is_first_transmit_player = false;
			}
			_second_resurrect = 0;
			TransmitResurrectPosPlayer(pPlane);
		}
	}
}
