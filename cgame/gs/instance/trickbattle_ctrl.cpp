
#include "../world.h"
#include "../worldmanager.h"
#include "../player_imp.h"
#include "trickbattle_ctrl.h"


void trickbattle_ctrl::Tick(world * pPlane)
{
	spin_autolock l(_lock);
	if((++ _tick_counter) % 20 != 0) return;

	CheckBattleResult(pPlane);

	if(_battle_end_timer > 0)
	{
		if(-- _battle_end_timer == 0)
		{
			SendDSBattleScore();
			SendDSBattleEnd();
		}
	}

	if(!_battle_result && _tick_counter % 2400 == 0)
	{
		SendDSBattleScore();
	}
}

void trickbattle_ctrl::BattleFactionSay(int faction, const void * buf, size_t size, char emote_id, const void * aux_data, size_t dsize, int self_id, int self_level)
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

void trickbattle_ctrl::UpdatePersonalScore(int roleid, int kill, int death, int score)
{
	spin_autolock keeper(_lock_pscore_map);
	PSCORE_MAP::iterator it = _pscore_map.find(roleid);
	if(it != _pscore_map.end())
	{
		personal_score & ps = it->second;
		ps.kill_count += kill;
		ps.death_count += death;
		ps.score += score;
		ps.changed = true;
	}
}

void trickbattle_ctrl::CheckBattleResult(world * pPlane)
{
	if(_battle_result) return; 
	
	int timestamp = g_timer.get_systime();
	if(timestamp > _data.end_timestamp)
	{
		_battle_result = 1;
		BattleEnd(pPlane);
		return;
	}
}

void trickbattle_ctrl::BattleEnd(world * pPlane)
{
	//pPlane->w_end_timestamp = g_timer.get_systime() + 120;
	//pPlane->w_destroy_timestamp = pPlane->w_end_timestamp + 300;
	pPlane->w_destroy_timestamp = g_timer.get_systime() + 30;

	pPlane->w_battle_result = _battle_result;
	//延迟5秒向delivery server发送battleend，此段时间用于玩家更新数据 
	_battle_end_timer = 5;
}

void trickbattle_ctrl::SendDSBattleScore()
{
	abase::vector<GMSV::TBPersonalScore, abase::fast_alloc<> > list;
	{
		spin_autolock l(_lock_pscore_map);
		list.reserve(_pscore_map.size());
		for(PSCORE_MAP::iterator it=_pscore_map.begin(); it!=_pscore_map.end(); ++it)
		{
			if(!it->second.changed) continue;
			
			personal_score & ps = it->second;
			GMSV::TBPersonalScore score;
			score.roleid 		= it->first;
			score.kill_count 	= ps.kill_count;
			score.death_count 	= ps.death_count;
			score.score			= ps.score;
			list.push_back(score);
			
			memset(&ps, 0, sizeof(ps));
		}
	}
	if(list.size())
		GMSV::SendTrickBattlePersonalScore(_data.battle_id, world_manager::GetWorldTag(), list.begin(), list.size());
}

void trickbattle_ctrl::SendDSBattleEnd()
{
	GMSV::SendTrickBattleEnd(_data.battle_id, world_manager::GetWorldTag());
}

void trickbattle_ctrl::PlayerEnter(gplayer * pPlayer,int type)
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

	if(type & 0x03)
	{
		spin_autolock keeper(_lock_pscore_map);
		PSCORE_MAP::iterator it = _pscore_map.find(pPlayer->ID.id);
		if(it == _pscore_map.end())
		{
			personal_score ps;
			memset(&ps, 0, sizeof(ps));
			_pscore_map.insert(PSCORE_MAP::value_type(pPlayer->ID.id, ps));
		}
	}
}

void trickbattle_ctrl::PlayerLeave(gplayer * pPlayer,int type)
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
