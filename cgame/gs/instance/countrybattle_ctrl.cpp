
#include "../world.h"
#include "../worldmanager.h"
#include "../player_imp.h"
#include "countrybattle_manager.h"
#include "countrybattle_ctrl.h"

#define BTYPE_FLAG		countrybattle_world_manager::BATTLE_TYPE_FLAG
#define BTYPE_TOWER		countrybattle_world_manager::BATTLE_TYPE_TOWER
#define BTYPE_STRONGHOLD	countrybattle_world_manager::BATTLE_TYPE_STRONGHOLD

#define SH_STATE_ATTACKER 			countrybattle_world_manager::STRONGHOLD_STATE_ATTACKER
#define SH_STATE_ATTACKER_HALF		countrybattle_world_manager::STRONGHOLD_STATE_ATTACKER_HALF
#define SH_STATE_NEUTRAL 			countrybattle_world_manager::STRONGHOLD_STATE_NEUTRAL
#define SH_STATE_DEFENDER_HALF 		countrybattle_world_manager::STRONGHOLD_STATE_DEFENDER_HALF
#define SH_STATE_DEFENDER 			countrybattle_world_manager::STRONGHOLD_STATE_DEFENDER
#define SH_STATE_COUNT				countrybattle_world_manager::STRONGHOLD_STATE_COUNT

void countrybattle_ctrl::Init(int battle_type)
{
	_data.battle_type = battle_type;
	switch(battle_type)
	{
		case BTYPE_FLAG: FlagModeInit(); break;
		case BTYPE_TOWER: TowerModeInit(); break;
		case BTYPE_STRONGHOLD: StrongholdModeInit(); break;
	}
}

void countrybattle_ctrl::FlagModeInit()
{
	_defence_score.goal = 20;
	_offense_score.goal = 20;
}

void countrybattle_ctrl::TowerModeInit()
{
	countrybattle_world_manager * pManager = (countrybattle_world_manager *)world_manager::GetInstance();
	size_t count = pManager->GetTowerList(true).size();
	ASSERT(count);
	_attacker_available_tower.insert(_attacker_available_tower.end(), count, 1);	//每个防御塔可以重复激活1次
	_defence_score.goal = count;	//攻方塔的数量即为守方目标
	count = pManager->GetTowerList(false).size();
	ASSERT(count);
	_defender_available_tower.insert(_defender_available_tower.end(), count, 1);	//每个防御塔可以重复激活1次
	_offense_score.goal = count;	//守方塔的数量即为攻方目标
}

void countrybattle_ctrl::StrongholdModeInit()
{
	_defence_score.goal = 99;
	_offense_score.goal = 99;
	countrybattle_world_manager * pManager = (countrybattle_world_manager *)world_manager::GetInstance();
	const countrybattle_world_manager::STRONGHOLD_LIST & list = pManager->GetStrongholdList();
	size_t count = list.size();
	ASSERT(count);
	_stronghold_state.insert(_stronghold_state.end(), count, abase::pair<int, int>(SH_STATE_NEUTRAL, 0));
}

void countrybattle_ctrl::UpdateExpiredDeathInfo(DEATH_INFO_LIST& death_list, int& lock)
{
	spin_autolock keeper(lock);
	__UpdateExpiredDeathInfoWithoutLock(death_list);
}

void countrybattle_ctrl::UpdateScoreRankList(world* pPlane, const cs_user_map& player_map, int& player_lock, const PSCORE_MAP& score_map, int& score_lock, score_rank_info& rank_info)
{
	abase::vector<int> players;
	{
		spin_autolock l(player_lock);	
		for(cs_user_map::const_iterator it = player_map.begin();it != player_map.end(); ++it)
		{
			const cs_user_list& list = it->second;
			for(unsigned int i = 0; i < list.size(); ++i)
			{
				players.push_back(list[i].first);
			}
		}
	}
	
	{
		spin_autolock l(score_lock);
		SCORE_RANK_LIST& ranks = rank_info.ranks;
		ranks.clear();
		int min_val_idx = 0;
		for(unsigned int i = 0; i < players.size(); ++i)
		{
			int roleid = players[i];
			PSCORE_MAP::const_iterator it = score_map.find(roleid);
			if(it == score_map.end()) continue;

			const personal_score& ps = it->second;
			int val = (int)(ps.dmg_output_weighted + (0.0001f * ps.dmg_endure * ps.soulpower) + (0.1f * ps.kill_count_weighted) + (0.001f * ps.soulpower));

			score_rank_entry entry;
			entry.roleid = roleid;
			entry.rank_val = val;
			if(ranks.size() < RANK_MAX_SIZE)
			{
				world::object_info info;
				bool ret = pPlane->QueryObject(XID(GM_TYPE_PLAYER, roleid), info);
				if(!ret) continue;
				entry.pos = info.pos;
				
				ranks.push_back(entry);
				if(val <= ranks[min_val_idx].rank_val) min_val_idx = (ranks.size() -1);
			}
			else if(val > ranks[min_val_idx].rank_val)
			{
				world::object_info info;
				bool ret = pPlane->QueryObject(XID(GM_TYPE_PLAYER, roleid), info);
				if(!ret) continue;
				entry.pos = info.pos;
				
				ranks[min_val_idx] = entry;
				for(unsigned int k = 0; k < ranks.size(); ++k)
				{
					if(ranks[k].rank_val < val) min_val_idx = k;
				}
			}
		}
		rank_info.timestamp = g_timer.get_systime();
	}
}

void countrybattle_ctrl::Tick(world * pPlane)
{
	spin_autolock l(_lock);
	if(++ _tick_counter < 20) return;
	_tick_counter = 0; 

	switch(_data.battle_type)
	{
		case BTYPE_FLAG:	FlagModeHeartbeat(pPlane);break;
		case BTYPE_TOWER:	TowerModeHeartbeat(pPlane);break;
		case BTYPE_STRONGHOLD: StrongholdModeHeartbeat(pPlane);break;
	}
	
	if(++ _tick_rank_info > 30) //每30秒计算一下分数排行
	{
		UpdateScoreRankList(pPlane, _defender_list, _user_list_lock, _defence_pscore_map, _lock_defence_map, _defence_rank_info);
		UpdateExpiredDeathInfo(_defence_death_info, _lock_defence_map);
		
		UpdateScoreRankList(pPlane, _attacker_list, _user_list_lock, _offense_pscore_map, _lock_offense_map, _offense_rank_info);
		UpdateExpiredDeathInfo(_offense_death_info, _lock_offense_map);
		_tick_rank_info = 0;
	}
	
	CheckBattleResult(pPlane);

	if(_battle_end_timer > 0)
	{
		if(-- _battle_end_timer == 0 && _need_send_ds)
			SendDSBattleEnd();
	}
}

void countrybattle_ctrl::FlagModeHeartbeat(world * pPlane)
{
	if(_stage == STAGE_PREPARE)
	{
		if(-- _timeout == 0)
		{
			//进入旗帜争夺阶段
			_stage = STAGE_CONTEST;
			_flag_controller = world_manager::GetInstance()->GenerateFlag();	
			pPlane->TriggerSpawn(_flag_controller, false);
			BroadcastFlagMsg(FLAG_MSG_GENERATE, false);
			__PRINTF("====进入夺旗阶段 controller=%d\n",_flag_controller);
		}
	}
	else if(_stage == STAGE_CONTEST)
	{
	}
	else if(_stage == STAGE_CONVOY)
	{
		if(-- _timeout == 0)
		{
			_timeout = FLAG_CARRIER_CHECK_INTERVAL;

			if(_flag_carrier.state == 0)
			{
				//旗手不存在了，此阶段结束
				__PRINTF("====旗手消失，进入准备阶段roleid=%d\n",_flag_carrier.id.id);
				_stage = STAGE_PREPARE;
				_timeout = PREPARE_TIME;
				BroadcastFlagMsg(FLAG_MSG_LOST, _flag_carrier.offense);
				_flag_carrier.id = XID(-1,-1);	
			}
			else
			{
				//重置旗手状态
				_flag_carrier.state = 0;
			}
			BroadcastFlagCarrier();	
		}
	}
}

void countrybattle_ctrl::TowerModeHeartbeat(world * pPlane)
{
	if(!_tower_generated)
	{
		countrybattle_world_manager * pManager = (countrybattle_world_manager *)world_manager::GetInstance();
		{
			const countrybattle_world_manager::TOWER_LIST & list = pManager->GetTowerList(true);
			std::set<int> group_set;
			for(size_t i=0; i<list.size(); i++)
			{
				if(group_set.find(list[i].group) == group_set.end()) group_set.insert(list[i].group);
			}
			for(std::set<int>::iterator it=group_set.begin(); it!=group_set.end(); ++it)
			{
				GenerateTower(pPlane, true, *it);
			}
		}
		{
			const countrybattle_world_manager::TOWER_LIST & list = pManager->GetTowerList(false);
			std::set<int> group_set;
			for(size_t i=0; i<list.size(); i++)
			{
				if(group_set.find(list[i].group) == group_set.end()) group_set.insert(list[i].group);
			}
			for(std::set<int>::iterator it=group_set.begin(); it!=group_set.end(); ++it)
			{
				GenerateTower(pPlane, false, *it);
			}
		}
		_tower_generated = true;
	}
}

void countrybattle_ctrl::StrongholdModeHeartbeat(world * pPlane)
{
	countrybattle_world_manager * pManager = (countrybattle_world_manager *)world_manager::GetInstance();
	const countrybattle_world_manager::STRONGHOLD_LIST & list = pManager->GetStrongholdList();
	if(!_stronghold_init)
	{
		for(size_t i=0; i<list.size(); i++)
		{
			pPlane->TriggerSpawn(list[i].data[SH_STATE_NEUTRAL].controller_id, false);
		}
		_stronghold_init = true;
	}
	int attacker_score = 0, defender_score = 0;
	size_t attacker_occupy = 0, defender_occupy = 0;
	for(size_t i=0; i<_stronghold_state.size(); i++)
	{
		int & state = _stronghold_state[i].first;
		int & counter = _stronghold_state[i].second;
		switch(state)
		{
			case SH_STATE_ATTACKER:
				if(++counter % SH_OCCUPY_GAIN_INTERVAL == 0)
					attacker_score += 1;
				++ attacker_occupy;
				break;
			case SH_STATE_ATTACKER_HALF:
				if(++counter >= SH_FULL_OCCUPY_TIME)
				{
					pPlane->ClearSpawn(list[i].data[SH_STATE_ATTACKER_HALF].controller_id, false);		
					pPlane->TriggerSpawn(list[i].data[SH_STATE_ATTACKER].controller_id, false);
					state = SH_STATE_ATTACKER;
					counter = 0;
					++ attacker_occupy;
					BroadcastStrongholdState();
				}
				break;
			case SH_STATE_DEFENDER_HALF:
				if(++counter >= SH_FULL_OCCUPY_TIME)
				{
					pPlane->ClearSpawn(list[i].data[SH_STATE_DEFENDER_HALF].controller_id, false);		
					pPlane->TriggerSpawn(list[i].data[SH_STATE_DEFENDER].controller_id, false);
					state = SH_STATE_DEFENDER;
					counter = 0;
					++ defender_occupy;
					BroadcastStrongholdState();
				}
				break;
			case SH_STATE_DEFENDER:
				if(++counter % SH_OCCUPY_GAIN_INTERVAL == 0)
					defender_score += 1;
				 ++ defender_occupy;
				break;
		}
	}
	if(attacker_score)
	{
		if(attacker_occupy == list.size()) attacker_score *= 3;
		interlocked_add(&_offense_score.score, attacker_score);
	}
	if(defender_score)
	{
		if(defender_occupy == list.size()) defender_score *= 3;
		interlocked_add(&_defence_score.score, defender_score);
	}
}

void countrybattle_ctrl::BattleFactionSay(int faction, const void * buf, size_t size, char emote_id, const void * aux_data, size_t dsize, int self_id, int self_level)
{
	spin_autolock keeper(_user_list_lock);
	if(faction & FACTION_BATTLEOFFENSE)
	{
		multi_send_chat_msg(_attacker_list,buf,size,GMSV::CHAT_CHANNEL_BATTLE,emote_id,aux_data,dsize,self_id,self_level);	
	}
	else
	{
		multi_send_chat_msg(_defender_list,buf,size,GMSV::CHAT_CHANNEL_BATTLE,emote_id,aux_data,dsize,self_id,self_level);	
	}
}

void countrybattle_ctrl::UpdatePersonalScore(bool offense, int roleid, int combat_time, int attend_time, int dmg_output, int dmg_output_weighted, int dmg_endure, int dmg_output_npc)
{
	if(offense)
		__UpdatePScoreTime(_offense_pscore_map,_lock_offense_map,roleid,combat_time,attend_time,dmg_output,dmg_output_weighted,dmg_endure,dmg_output_npc);
	else
		__UpdatePScoreTime(_defence_pscore_map,_lock_defence_map,roleid,combat_time,attend_time,dmg_output,dmg_output_weighted,dmg_endure,dmg_output_npc);
}

void countrybattle_ctrl::OnPlayerDeath(gplayer * pPlayer, const XID & killer, int player_soulpower, const A3DVECTOR& pos)
{
	if(pPlayer->IsBattleOffense())
	{
		__UpdatePScoreDeath(_offense_pscore_map, _offense_death_info, _lock_offense_map, pPlayer->ID.id, pos);	
		interlocked_increment(&_offense_score.death_count);
		if(killer.IsPlayer())
		{
			__UpdatePScoreKill(_defence_pscore_map,_lock_defence_map,killer.id,player_soulpower);
			interlocked_increment(&_defence_score.kill_count);
		}
	}
	else
	{
		__UpdatePScoreDeath(_defence_pscore_map, _defence_death_info, _lock_defence_map, pPlayer->ID.id, pos);
		interlocked_increment(&_defence_score.death_count);
		if(killer.IsPlayer())
		{
			__UpdatePScoreKill(_offense_pscore_map,_lock_offense_map,killer.id,player_soulpower);
			interlocked_increment(&_offense_score.kill_count);
		}
	}
}

bool countrybattle_ctrl::PickUpFlag(gplayer * pPlayer)
{
	if(_data.battle_type != BTYPE_FLAG) return false;
	ASSERT(pPlayer->spinlock);
	spin_autolock keeper(_lock);
	if(_stage == STAGE_CONTEST)
	{
		if(pPlayer->IsBattleOffense())
		{
			interlocked_add(&_offense_score.score, 1);
			__UpdatePScoreScore(_offense_pscore_map,_lock_offense_map,pPlayer->ID.id,1);
		}
		else
		{
			interlocked_add(&_defence_score.score, 1);
			__UpdatePScoreScore(_defence_pscore_map,_lock_defence_map,pPlayer->ID.id,1);
		}
		
		_stage = STAGE_CONVOY;
		_timeout = FLAG_CARRIER_CHECK_INTERVAL;
		ASSERT(_flag_carrier.id.id <= 0);
		_flag_carrier.id = pPlayer->ID;
		_flag_carrier.pos = pPlayer->pos;
		_flag_carrier.offense = pPlayer->IsBattleOffense();
		_flag_carrier.state = 1;
		((gplayer_imp *)pPlayer->imp)->SetFlagCarrier(true);
		pPlayer->plane->ClearSpawn(_flag_controller, false);
		BroadcastFlagCarrier();
		BroadcastFlagMsg(FLAG_MSG_PICKUP, _flag_carrier.offense);
		__PRINTF("====夺旗成功 进入护送阶段role=%d\n",_flag_carrier.id.id);
		return true;
	}
	return false;
}

bool countrybattle_ctrl::HandInFlag(gplayer * pPlayer)
{
	if(_data.battle_type != BTYPE_FLAG) return false;
	ASSERT(pPlayer->spinlock);
	spin_autolock keeper(_lock);
	if(_stage == STAGE_CONVOY && _flag_carrier.id == pPlayer->ID)
	{
		if(pPlayer->IsBattleOffense())
		{
			interlocked_add(&_offense_score.score, 10);
			__UpdatePScoreScore(_offense_pscore_map,_lock_offense_map,pPlayer->ID.id,10);
		}
		else
		{
			interlocked_add(&_defence_score.score, 10);
			__UpdatePScoreScore(_defence_pscore_map,_lock_defence_map,pPlayer->ID.id,10);
		}
		
		_stage = STAGE_PREPARE;
		_timeout = PREPARE_TIME;
		BroadcastFlagMsg(FLAG_MSG_HANDIN, _flag_carrier.offense);
		_flag_carrier.id = XID(-1,-1);	
		((gplayer_imp *)pPlayer->imp)->SetFlagCarrier(false);
		BroadcastFlagCarrier();
		__PRINTF("====交旗成功 进入准备阶段role=%d\n",pPlayer->ID.id);
		return true;
	}
	return false;
}

void countrybattle_ctrl::UpdateFlagCarrier(int roleid, const A3DVECTOR & pos)
{
	if(_data.battle_type != BTYPE_FLAG) return;
	spin_autolock keeper(_lock);
	if(_stage == STAGE_CONVOY && _flag_carrier.id.id == roleid)
	{
		_flag_carrier.pos = pos;
		_flag_carrier.state = 1;
	}
}

void countrybattle_ctrl::OnTowerDestroyed(world * pPlane, bool offense, int tid)
{
	if(_data.battle_type != BTYPE_TOWER) return;
	spin_autolock keeper(_lock);
	if(offense)
	{
		interlocked_add(&_defence_score.score, 1);
	}
	else
	{
		interlocked_add(&_offense_score.score, 1);
	}
	countrybattle_world_manager * pManager = (countrybattle_world_manager *)world_manager::GetInstance();
	const countrybattle_world_manager::TOWER_LIST & list = pManager->GetTowerList(offense);
	for(size_t i=0; i<list.size(); i++)
	{
		if(list[i].npc_tid == tid)
		{
			GenerateTower(pPlane, offense, list[i].group);
		}
	}
}

void countrybattle_ctrl::OccupyStrongHold(int mine_tid, gplayer* pPlayer)
{
	if(_data.battle_type != BTYPE_STRONGHOLD) return;
	spin_autolock keeper(_lock);
	countrybattle_world_manager * pManager = (countrybattle_world_manager *)world_manager::GetInstance();
	const countrybattle_world_manager::STRONGHOLD_LIST & list = pManager->GetStrongholdList();
	for(size_t i=0; i<list.size(); i++)
	{
		for(size_t s=0; s<SH_STATE_COUNT; s++)
		{
			if(list[i].data[s].mine_tid == mine_tid)
			{
				pPlayer->plane->ClearSpawn(list[i].data[s].controller_id, false);
				if(pPlayer->IsBattleOffense())
				{
					ASSERT(s == SH_STATE_DEFENDER || s == SH_STATE_DEFENDER_HALF || s == SH_STATE_NEUTRAL);
					pPlayer->plane->TriggerSpawn(list[i].data[SH_STATE_ATTACKER_HALF].controller_id, false);
					_stronghold_state[i].first = SH_STATE_ATTACKER_HALF;
					_stronghold_state[i].second = 0;
					__UpdatePScoreScore(_offense_pscore_map,_lock_offense_map,pPlayer->ID.id,1);
				}
				else
				{
					ASSERT(s == SH_STATE_ATTACKER || s == SH_STATE_ATTACKER_HALF || s == SH_STATE_NEUTRAL);
					pPlayer->plane->TriggerSpawn(list[i].data[SH_STATE_DEFENDER_HALF].controller_id, false);
					_stronghold_state[i].first = SH_STATE_DEFENDER_HALF;
					_stronghold_state[i].second = 0;
					__UpdatePScoreScore(_defence_pscore_map,_lock_defence_map,pPlayer->ID.id,1);
				}
				BroadcastStrongholdState();
			}
		}
	}

}

bool countrybattle_ctrl::GetStrongholdNearby(bool offense, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag)
{
	if(_data.battle_type != BTYPE_STRONGHOLD) return false;
	spin_autolock keeper(_lock);
	countrybattle_world_manager * pManager = (countrybattle_world_manager *)world_manager::GetInstance();
	const countrybattle_world_manager::STRONGHOLD_LIST & list = pManager->GetStrongholdList();
	for(size_t i=0; i<_stronghold_state.size(); i++)
	{
		if(offense)
		{
			if(_stronghold_state[i].first != SH_STATE_ATTACKER)	continue;
		}
		else
		{
			if(_stronghold_state[i].first != SH_STATE_DEFENDER) continue;
		}

		if(list[i].squared_radius > 1e-6 && opos.squared_distance(list[i].pos) <= list[i].squared_radius)
		{
			pos = list[i].pos;
			tag = world_manager::GetWorldTag();
			return true;
		}
	}
	return false;
}

bool countrybattle_ctrl::GetPersonalScore(bool offense, int roleid, int& combat_time, int& attend_time, int& kill_count, int& death_count, int& country_kill_count,int& country_death_count)
{
	bool rst;
	if(offense)
		rst = __GetPScore(_offense_pscore_map,_lock_offense_map,roleid,combat_time,attend_time,kill_count,death_count);
	else
		rst = __GetPScore(_defence_pscore_map,_lock_defence_map,roleid,combat_time,attend_time,kill_count,death_count);
	if(rst)
	{
		if(offense)
		{
			country_kill_count = _offense_score.kill_count;
			country_death_count = _offense_score.death_count;
		}
		else
		{
			country_kill_count = _defence_score.kill_count;
			country_death_count = _defence_score.death_count;
		}
	}
	return rst;
}

bool countrybattle_ctrl::GetCountryBattleInfo(int & attacker_count, int & defender_count)
{
	attacker_count = _data.attacker_count;
	defender_count = _data.defender_count;
	return true;
}

void countrybattle_ctrl::GetStongholdState(int roleid, int cs_index, int cs_sid)
{
	if(_data.battle_type != BTYPE_STRONGHOLD) return;

	packet_wrapper h1(32);
	using namespace S2C;
	CMD::Make<CMD::countrybattle_stronghold_state_notify>::From(h1, _stronghold_state.size());
	for(size_t i=0; i<_stronghold_state.size(); i++)
		CMD::Make<CMD::countrybattle_stronghold_state_notify>::AddState(h1, _stronghold_state[i].first);
	
	send_ls_msg(cs_index,roleid,cs_sid,h1);
}

void countrybattle_ctrl::MakeLiveResultData(packet_wrapper& h1, world* pPlane, int& lock, score_rank_info& rank_info, DEATH_INFO_LIST& death_list)
{
	spin_autolock l(lock);
	
	__UpdateRankInfoCurPosWithoutLock(pPlane, rank_info);
	h1 << (unsigned int)rank_info.ranks.size();
	for(unsigned int i = 0; i < rank_info.ranks.size(); ++i)
	{
		h1 << rank_info.ranks[i].roleid << rank_info.ranks[i].rank_val << rank_info.ranks[i].pos;
	}
	
	__UpdateExpiredDeathInfoWithoutLock(death_list);
	h1 << (unsigned int)death_list.size();
	for(unsigned int i = 0; i < death_list.size(); ++i)
	{
		h1 << death_list[i].roleid << death_list[i].timestamp << death_list[i].pos;
	}
}

bool countrybattle_ctrl::GetLiveShowResult(int roleid, int cs_index, int cs_sid, world* pPlane)
{
	packet_wrapper h1;
	using namespace S2C;
	CMD::Make<CMD::countrybattle_live_result>::From(h1);

	MakeLiveResultData(h1, pPlane, _lock_defence_map, _defence_rank_info, _defence_death_info);
	MakeLiveResultData(h1, pPlane, _lock_offense_map, _offense_rank_info, _offense_death_info);

	send_ls_msg(cs_index,roleid,cs_sid,h1);
	return true;
}

void countrybattle_ctrl::CheckBattleResult(world * pPlane)
{
	if(_battle_result) return; 
	
	pPlane->w_defence_goal = _defence_score.goal; 
	pPlane->w_defence_cur_score = _defence_score.score;
	if(_defence_score.score >= _defence_score.goal)
	{
		_battle_result = BR_WINNER_DEFENCE;
		BattleEnd(pPlane);
		return;
	}

	pPlane->w_offense_goal = _offense_score.goal;
	pPlane->w_offense_cur_score = _offense_score.score;
	if(_offense_score.score >= _offense_score.goal)
	{
		_battle_result = BR_WINNER_OFFENSE;
		BattleEnd(pPlane);
		return;
	}
	
	int timestamp = g_timer.get_systime();
	if(timestamp > _data.end_timestamp)
	{
		if(_defence_score.score >= _offense_score.score)
			_battle_result = BR_WINNER_DEFENCE;
		else
			_battle_result = BR_WINNER_OFFENSE;
		BattleEnd(pPlane);
		return;
	}
}

void countrybattle_ctrl::BattleEnd(world * pPlane)
{
	//pPlane->w_end_timestamp = g_timer.get_systime() + 120;
	//pPlane->w_destroy_timestamp = pPlane->w_end_timestamp + 300;
	pPlane->w_destroy_timestamp = g_timer.get_systime() + 30;

	pPlane->w_battle_result = _battle_result;
	//延迟5秒向delivery server发送battleend，此段时间用于玩家更新数据 
	_battle_end_timer = 5;
}

void countrybattle_ctrl::SendDSBattleEnd()
{
	abase::vector<GMSV::CBPersonalScore> defender_list;
	abase::vector<GMSV::CBPersonalScore> attacker_list;
	{
		spin_autolock l(_lock_defence_map);
		defender_list.reserve(_defence_pscore_map.size());
		for(PSCORE_MAP::iterator it=_defence_pscore_map.begin(); it!=_defence_pscore_map.end(); ++it)
		{
			const personal_score & ps = it->second;
			GMSV::CBPersonalScore score;
			score.roleid 		= it->first;
			score.cls 			= ps.cls;
			score.minor_strength= ps.soulpower;
			score.combat_time 	= ps.combat_time;
			score.attend_time 	= ps.attend_time;
			score.kill_count 	= ps.kill_count;
			score.death_count 	= ps.death_count;
			score.score			= ps.score;
			score.dmg_output	= ps.dmg_output;
			score.dmg_output_weighted= ps.dmg_output_weighted;
			score.dmg_endure	= ps.dmg_endure;
			score.kill_count_weighted= ps.kill_count_weighted;
			if(_data.battle_type == BTYPE_TOWER)
			{
				int s = ps.dmg_output_npc/50000;
				score.score += s>30 ? 30 : s;
			}
			else if(_data.battle_type == BTYPE_STRONGHOLD)
			{
				if(score.score > 30) score.score = 30;
			}
			defender_list.push_back(score);
		}
	}
	{
		spin_autolock l(_lock_offense_map);
		attacker_list.reserve(_offense_pscore_map.size());
		for(PSCORE_MAP::iterator it=_offense_pscore_map.begin(); it!=_offense_pscore_map.end(); ++it)
		{
			const personal_score & ps = it->second;
			GMSV::CBPersonalScore score;
			score.roleid 		= it->first;
			score.cls 			= ps.cls;
			score.minor_strength= ps.soulpower;
			score.combat_time 	= ps.combat_time;
			score.attend_time 	= ps.attend_time;
			score.kill_count 	= ps.kill_count;
			score.death_count 	= ps.death_count;
			score.score			= ps.score;
			score.dmg_output	= ps.dmg_output;
			score.dmg_output_weighted= ps.dmg_output_weighted;
			score.dmg_endure	= ps.dmg_endure;
			score.kill_count_weighted= ps.kill_count_weighted;
			if(_data.battle_type == BTYPE_TOWER)
			{
				int s = ps.dmg_output_npc/50000;
				score.score += s>30 ? 30 : s;
			}
			else if(_data.battle_type == BTYPE_STRONGHOLD)
			{
				if(score.score > 30) score.score = 30;
			}
			attacker_list.push_back(score);
		}
	}
	GMSV::SendCountryBattleEnd(_data.battle_id, _battle_result, _data.country_defender, _data.country_attacker, defender_list.begin(), defender_list.size(), attacker_list.begin(), attacker_list.size());
}

void countrybattle_ctrl::BroadcastFlagCarrier()
{
	packet_wrapper h1(32);
	using namespace S2C;
	CMD::Make<CMD::countrybattle_flag_carrier_notify>::From(h1, _flag_carrier.id.id, _flag_carrier.pos, (_flag_carrier.offense ? 1 : 0));

	spin_autolock keeper(_user_list_lock);
	multi_send_ls_msg(_all_list, h1, _flag_carrier.id.id);
}

void countrybattle_ctrl::BroadcastFlagMsg(int msg, bool offense)
{
	packet_wrapper h1(16);
	using namespace S2C;
	CMD::Make<CMD::countrybattle_flag_msg_notify>::From(h1, msg, (offense ? 1 : 0));

	spin_autolock keeper(_user_list_lock);
	multi_send_ls_msg(_all_list, h1);
}

void countrybattle_ctrl::BroadcastStrongholdState()
{
	packet_wrapper h1(32);
	using namespace S2C;
	CMD::Make<CMD::countrybattle_stronghold_state_notify>::From(h1, _stronghold_state.size());
	for(size_t i=0; i<_stronghold_state.size(); i++)
		CMD::Make<CMD::countrybattle_stronghold_state_notify>::AddState(h1, _stronghold_state[i].first);
	
	spin_autolock keeper(_user_list_lock);
	multi_send_ls_msg(_all_list, h1);
}

bool countrybattle_ctrl::GenerateTower(world * pPlane, bool offense, int group)
{
	countrybattle_world_manager * pManager = (countrybattle_world_manager *)world_manager::GetInstance();
	const countrybattle_world_manager::TOWER_LIST & tower_list = pManager->GetTowerList(offense);
	abase::vector<char> * pList = NULL;
	if(offense)
	{
		pList = &_attacker_available_tower;
	}
	else
	{
		pList = &_defender_available_tower;
	}
	int ilist[pList->size()];
	int icount = 0;
	for(size_t i=0; i<pList->size(); i++)
	{
		if((*pList)[i] > 0 && tower_list[i].group == group)
		{
			ilist[icount++] = i;
		}
	}
	if(icount == 0) return false;
	int index = ilist[ abase::Rand(0, icount-1) ];
	pPlane->ClearSpawn(tower_list[index].controller_id, false);
	pPlane->TriggerSpawn(tower_list[index].controller_id, false);
	-- (*pList)[index];
	return true;
}

void countrybattle_ctrl::PlayerEnter(gplayer * pPlayer,int type)
{
	{
		spin_autolock keeper(_user_list_lock);
		AddMapNode(_all_list,pPlayer);
		if(type & 0x01)
		{
			//attacker
			AddMapNode(_attacker_list,pPlayer);
		}
		else if(type & 0x02)
		{
			//defender
			AddMapNode(_defender_list,pPlayer);
		}
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	if(type & 0x01)
	{
		__TryInsertPScoreMap(_offense_pscore_map, _lock_offense_map, pPlayer->ID.id, pImp->GetPlayerClass(), pImp->GetSoulPower());
	}
	else if(type & 0x02)
	{
		__TryInsertPScoreMap(_defence_pscore_map, _lock_defence_map, pPlayer->ID.id, pImp->GetPlayerClass(), pImp->GetSoulPower());
	}
}

void countrybattle_ctrl::PlayerLeave(gplayer * pPlayer,int type)
{
	spin_autolock keeper(_user_list_lock);
	DelMapNode(_all_list,pPlayer);
	if(type & 0x01)
	{
		//attacker
		DelMapNode(_attacker_list,pPlayer);
	}
	else if(type & 0x02)
	{
		//defender
		DelMapNode(_defender_list,pPlayer);
	}
}

void countrybattle_ctrl::DestroyCountryBattle(world* pPlane)
{
	spin_autolock l(_lock);
	_need_send_ds = false;
	_data.end_timestamp = g_timer.get_systime();
}

