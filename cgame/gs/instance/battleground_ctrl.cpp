#include "battleground_ctrl.h"
#include "../faction.h"
#include "../obj_interface.h"
#include <gsp_if.h>
#include <factionlib.h>

void battleground_ctrl::PlayerEnter(gplayer * pPlayer,int type)
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

void battleground_ctrl::PlayerLeave(gplayer * pPlayer,int type)
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

void 
battleground_ctrl::DestroyKeyBuilding(int faction)
{
	if(_battle_result) return;
	map_data *pData = NULL;
	if(faction & (FACTION_BATTLEOFFENSE | FACTION_OFFENSE_FRIEND))
	{
		//攻方 
		pData = &_offense_data;
	}       
	else    
	{       
		//非攻方即认为守方
		pData = &_defence_data;
	}

	interlocked_increment(&pData->key_building);
}

void
battleground_ctrl::DestroyMobs(int faction)
{
	if(_battle_result) return;
	map_data *pData = NULL;
	if(faction & (FACTION_BATTLEOFFENSE | FACTION_OFFENSE_FRIEND))
	{
		//攻方 
		pData = &_offense_data;
	}       
	else    
	{       
		//非攻方即认为守方
		pData = &_defence_data;
	}

	interlocked_increment(&pData->mobs);
}

void 
battleground_ctrl::Tick(world * pPlane)
{
	CheckBattleResult(pPlane);
}

int 
battleground_ctrl::GetGoal(const battleground_ctrl::map_data &data)
{
	switch(_win_condition)
	{
		case 0:
			//取得中心建筑
			return data.key_building;
		case 1:
			//取得怪物数量
			return data.mobs;
		default:
			return 0;
	}
}

void 
battleground_ctrl::CheckBattleResult(world * pPlane)
{
	if(_battle_result) return;
	int cur_goal = GetGoal(_defence_data);
	int goal = GetGoal(_defence_init_data);
	pPlane->w_defence_goal = goal;
	pPlane->w_defence_cur_score = cur_goal;
	if(goal && cur_goal >= goal) 
	{
		_battle_result = BR_WINNER_OFFENSE;
		BattleEnd(pPlane);
		return ;
	}

	cur_goal = GetGoal(_offense_data);
	goal = GetGoal(_offense_init_data);
	pPlane->w_offense_goal = goal;
	pPlane->w_offense_cur_score = cur_goal;
	if(goal && cur_goal >= goal) 
	{
		_battle_result = BR_WINNER_DEFENCE;
		BattleEnd(pPlane);
		return ;
	}

	int timestamp = g_timer.get_systime();
	if(timestamp > _data.end_timestamp)
	{
		_battle_result = BR_TIMEOUT;
		BattleEnd(pPlane);
		return ;
	}
}

void 
battleground_ctrl::BattleEnd(world * pPlane)
{
	//通知服务器
	GNET::SendBattleEnd(_data.battle_id,_battle_result,_data.faction_defender,_data.faction_attacker);

	//设置结束时间
	pPlane->w_end_timestamp = g_timer.get_systime() + 120;
	pPlane->w_destroy_timestamp = pPlane->w_end_timestamp + 300;
	
	//同步战斗结果至world 中 这步必须最后完成
	pPlane->w_battle_result = _battle_result;

}

void 
battleground_ctrl::BattleFactionSay(int faction, const void * buf, size_t size, char emote_id, const void * aux_data, size_t dsize, int self_id, int self_level)
{
	if(faction & FACTION_OFFENSE_FRIEND)
	{
		//攻方
		multi_send_chat_msg(_attacker_list,buf,size,GMSV::CHAT_CHANNEL_FACTION,emote_id,aux_data,dsize,self_id,self_level);
	}
	else if(faction & FACTION_DEFENCE_FRIEND)
	{
		multi_send_chat_msg(_defender_list,buf,size,GMSV::CHAT_CHANNEL_FACTION,emote_id,aux_data,dsize,self_id,self_level);
	}
}

void 
battleground_ctrl::BattleSay(const void * buf, size_t size)
{
	multi_send_chat_msg(_all_list,buf,size,GMSV::CHAT_CHANNEL_FACTION,0,0,0,0,0);
}

