#include "clstab.h"
#include "faction.h"
#include "world.h"
#include "player_imp.h"
#include "playerbattlebase.h"
#include "playertrickbattle.h"
#include "instance/trickbattle_ctrl.h"
#include "invincible_filter.h"

DEFINE_SUBSTANCE(gplayer_trickbattle,gplayer_battle_base,CLS_TRICKBATTLE_PLAYER_IMP)

namespace
{
	const static int max_multikill_score = 10;

	int __multikill_score(int kill)
	{
		//连续击杀得分是0.1乘以连续击杀人数的平方并四舍五入取整，然后下限为1，上限为10
		if (kill > 10000) return max_multikill_score;	//防止乘法导致溢出
		int score = (int)(0.1f*kill*kill + 0.5f);
		if (score <= 0) score = 1;
		else if (score > max_multikill_score) score = max_multikill_score;
		return score;
	}
	int __level_sub_score(int self_chariot_lv, int kill_chariot_lv)
	{
		int lv_sub = kill_chariot_lv - self_chariot_lv;
		if (lv_sub > 30) lv_sub = 30;
		else if (lv_sub <= 0) return 1;	//杀死自己同级或者低级的战车，只得1分
		return 1<<lv_sub;	//得分是2的等级差次方
	}
	int __calc_score(int kill, int self_chariot_lv, int kill_chariot_lv)
	{
		//计算本次击杀得分，用连续击杀得分加上等级差得分
		return __multikill_score(kill) + __level_sub_score(self_chariot_lv,kill_chariot_lv);
	}
}

void gplayer_trickbattle::OnHeartbeat(size_t tick)
{
	if(_parent->IsZombie())
	{           
		if(++ death_couter >= 30)
		{
			SendTo<0>(GM_MSG_GM_RESURRECT,_parent->ID,last_primary_chariot);
		}
	} 

	gplayer_battle_base::OnHeartbeat(tick);

	if(++ sync_counter >= 60)
	{
		sync_counter = 0;
		SyncScoreToPlane();
	}

	if(notify_client)
	{
		_runner->trickbattle_personal_score(score_total.kill, score_total.death, score_total.score, multi_kill);
		notify_client = false;
	}
}

int gplayer_trickbattle::MessageHandler(world * pPlane ,const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_TRICKBATTLE_PLAYER_KILLED:
		{
			score_total.kill ++;
			score_delta.kill ++;
			multi_kill ++;

			int delta = __calc_score(multi_kill, chariot_lv, msg.param);
			score_total.score += delta;
			score_delta.score += delta;
			changed = true;
			notify_client = true;
		}
		return 0;
	}

	return gplayer_battle_base::MessageHandler(pPlane, msg);
}

void gplayer_trickbattle::PlayerEnterServer()
{
	gplayer_battle_base::PlayerEnterServer();
	
	trickbattle_ctrl * pCtrl = (trickbattle_ctrl *)_plane->w_ctrl;
	gplayer * pPlayer = GetParent();
	if(pPlayer->IsBattleOffense())
	{
		//攻方
		_runner->enter_trickbattle(1,pCtrl->_data.battle_id,pCtrl->_data.end_timestamp);
		pCtrl->PlayerEnter(pPlayer,0x01);
	}
	else if(pPlayer->IsBattleDefence())
	{
		//守方
		_runner->enter_trickbattle(2,pCtrl->_data.battle_id,pCtrl->_data.end_timestamp);
		pCtrl->PlayerEnter(pPlayer,0x02);
	}
	else
	{
		_runner->enter_trickbattle(0,pCtrl->_data.battle_id,pCtrl->_data.end_timestamp);
		pCtrl->PlayerEnter(pPlayer,0x0);
	}
	GMSV::SendTrickBattleEnter(_parent->ID.id, pCtrl->_data.battle_id, world_manager::GetWorldTag());
	
	_petman.RecallPet(this);
}
 
void gplayer_trickbattle::PlayerLeaveServer()
{
	trickbattle_ctrl * pCtrl = (trickbattle_ctrl *)_plane->w_ctrl;
	gplayer * pPlayer = GetParent();
	
	SyncScoreToPlane();

	if(pPlayer->IsBattleOffense()) 
	{
		pCtrl->DelAttacker();
		pCtrl->DelChariot(0x01,chariot_id);
		pCtrl->PlayerLeave(pPlayer,0x01);
	}
	else if(pPlayer->IsBattleDefence())
	{
		pCtrl->DelDefender();
		pCtrl->DelChariot(0x02,chariot_id);
		pCtrl->PlayerLeave(pPlayer,0x02);
	}
	else
	{
		pCtrl->PlayerLeave(pPlayer,0x0);	
	}

	gplayer_battle_base::PlayerLeaveServer();

	_runner->enter_trickbattle(0,0,0);
	GMSV::SendTrickBattleLeave(_parent->ID.id, pCtrl->_data.battle_id, world_manager::GetWorldTag());
}
 
void gplayer_trickbattle::PlayerLeaveWorld()
{
	trickbattle_ctrl * pCtrl = (trickbattle_ctrl *)_plane->w_ctrl;
	gplayer * pPlayer = GetParent();
	
	SyncScoreToPlane();

	if(pPlayer->IsBattleOffense()) 
	{
		pCtrl->DelAttacker();
		pCtrl->DelChariot(0x01,chariot_id);
		pCtrl->PlayerLeave(pPlayer,0x01);
	}
	else if(pPlayer->IsBattleDefence())
	{
		pCtrl->DelDefender();
		pCtrl->DelChariot(0x02,chariot_id);
		pCtrl->PlayerLeave(pPlayer,0x02);
	}
	else
	{
		pCtrl->PlayerLeave(pPlayer,0x0);	
	}
	
	gplayer_battle_base::PlayerLeaveWorld();
	
	_runner->enter_trickbattle(0,0,0);
	GMSV::SendTrickBattleLeave(_parent->ID.id, pCtrl->_data.battle_id, world_manager::GetWorldTag());
}

void gplayer_trickbattle::OnDeath(const XID & lastattack,bool is_pariah, char attacker_mode, int taskdead)
{
	gplayer_battle_base::OnDeath(lastattack,is_pariah,attacker_mode,taskdead);

	gplayer * pPlayer = GetParent();
	if(pPlayer->IsBattleOffense() || pPlayer->IsBattleDefence())
	{
		if(lastattack.IsPlayerClass()) SendTo<0>(GM_MSG_TRICKBATTLE_PLAYER_KILLED, lastattack, chariot_lv);
		score_total.death ++;
		score_delta.death ++;
		multi_kill = 0;
		changed = true;
		notify_client = true;
	}
}

bool gplayer_trickbattle::CanResurrect(int param)
{
	int chariot = param;
	DATA_TYPE dt;
	CHARIOT_CONFIG * cfg = (CHARIOT_CONFIG *)world_manager::GetDataMan().get_data_ptr(chariot, ID_SPACE_CONFIG, dt);
	if(!cfg || dt != DT_CHARIOT_CONFIG) return false;
	if(cfg->pre_chariot > 0) return false;	//只能选择初级战车

	return gplayer_battle_base::CanResurrect(param);
}

int gplayer_trickbattle::Resurrect(const A3DVECTOR & pos,bool nomove,float exp_reduce,int target_tag,float hp_factor, float mp_factor, int param)
{
	gplayer_battle_base::Resurrect(pos,nomove,exp_reduce,target_tag,hp_factor,mp_factor,param);
	//设置无敌
	_filters.AddFilter(new invincible_filter(this,FILTER_INVINCIBLE,5));
	//
	TrickBattleTransformChariot(param);
	death_couter = 0;
	return 0;
}

void gplayer_trickbattle::SyncScoreToPlane()
{
	if(!changed) return;

	gplayer * pPlayer = GetParent();
	if(pPlayer->IsBattleOffense() || pPlayer->IsBattleDefence()) 
	{
		if(_plane->w_ctrl)
		{
			_plane->w_ctrl->UpdatePersonalScore(_parent->ID.id, score_delta.kill, score_delta.death, score_delta.score);
			memset(&score_delta, 0, sizeof(SCORE));
		}
	}
	changed = false;
}

bool gplayer_trickbattle::TrickBattleTransformChariot(int chariot)
{
	DATA_TYPE dt;
	CHARIOT_CONFIG * cfg = (CHARIOT_CONFIG *)world_manager::GetDataMan().get_data_ptr(chariot, ID_SPACE_CONFIG, dt);
	ASSERT(cfg && dt == DT_CHARIOT_CONFIG);
	ASSERT(cfg->pre_chariot <= 0);
	ASSERT(cfg->level > 0);
	ASSERT(sizeof(cfg->magic_defences)/sizeof(cfg->magic_defences[0]) == 5 && sizeof(cfg->skill)/sizeof(cfg->skill[0]) == 4);

	_filters.ClearSpecFilter(filter::FILTER_MASK_DEBUFF | filter::FILTER_MASK_BUFF);
	_skill.SetChariotFilter(object_interface(this),cfg->shape,cfg->hp,cfg->defence,cfg->magic_defences,cfg->damage,cfg->magic_damage,cfg->speed,
			cfg->hp_inc_ratio,cfg->skill);
	_basic.hp = _cur_prop.max_hp;
	SetRefreshState();

	gplayer * pPlayer = GetParent();
	if(pPlayer->IsBattleOffense()) 
	{
		if(_plane->w_ctrl)
		{
			if (chariot_id != 0)
				_plane->w_ctrl->DelChariot(0x01,chariot_id);

			_plane->w_ctrl->AddChariot(0x01,chariot);
		}
	}
	else if(pPlayer->IsBattleDefence())
	{
		if(_plane->w_ctrl)
		{
			if (chariot_id != 0)
				_plane->w_ctrl->DelChariot(0x02,chariot_id);

			_plane->w_ctrl->AddChariot(0x02,chariot);
		}
	}
	
	chariot_id = chariot;
	chariot_lv = cfg->level;
	chariot_energy = 0;
	last_primary_chariot = chariot_id;
	_runner->trickbattle_chariot_info(chariot_id,chariot_energy);
	return true;
}

bool gplayer_trickbattle::TrickBattleUpgradeChariot(int chariot)
{
	DATA_TYPE dt;
	CHARIOT_CONFIG * cfg = (CHARIOT_CONFIG *)world_manager::GetDataMan().get_data_ptr(chariot, ID_SPACE_CONFIG, dt);
	if(!cfg || dt != DT_CHARIOT_CONFIG) return false;
	if(cfg->pre_chariot <= 0 || cfg->pre_chariot != chariot_id) return false;
	if(cfg->upgrade_cost > chariot_energy) return false;
	if(cfg->level <= 0)	return false;
	ASSERT(sizeof(cfg->magic_defences)/sizeof(cfg->magic_defences[0]) == 5 && sizeof(cfg->skill)/sizeof(cfg->skill[0]) == 4);

	_filters.ClearSpecFilter(filter::FILTER_MASK_DEBUFF | filter::FILTER_MASK_BUFF);
	_skill.SetChariotFilter(object_interface(this),cfg->shape,cfg->hp,cfg->defence,cfg->magic_defences,cfg->damage,cfg->magic_damage,cfg->speed,
			cfg->hp_inc_ratio,cfg->skill);
	_basic.hp = _cur_prop.max_hp;
	SetRefreshState();
	
	gplayer * pPlayer = GetParent();
	if(pPlayer->IsBattleOffense()) 
	{
		if(_plane->w_ctrl)
		{
			if (chariot_id != 0)
				_plane->w_ctrl->DelChariot(0x01,chariot_id);

			_plane->w_ctrl->AddChariot(0x01,chariot);
		}
	}
	else if(pPlayer->IsBattleDefence())
	{
		if(_plane->w_ctrl)
		{
			if (chariot_id != 0)
				_plane->w_ctrl->DelChariot(0x02,chariot_id);

			_plane->w_ctrl->AddChariot(0x02,chariot);
		}
	}
	
	chariot_id = chariot;
	chariot_lv = cfg->level;
	chariot_energy -= cfg->upgrade_cost;
	_runner->trickbattle_chariot_info(chariot_id,chariot_energy);
	return true;
}

void gplayer_trickbattle::TrickBattleIncChariotEnergy(int energy)
{
	chariot_energy += energy;
	if(chariot_energy > CHARIOT_ENERGY_MAX) chariot_energy = CHARIOT_ENERGY_MAX;
	_runner->trickbattle_chariot_info(chariot_id,chariot_energy);
}

void gplayer_trickbattle::QueryTrickBattleChariots()
{
	abase::hash_map<int, int> attacker_map;
	abase::hash_map<int, int> defender_map;

	if (_plane->w_ctrl)
	{
		_plane->w_ctrl->GetChariots(0x01,attacker_map);
		_plane->w_ctrl->GetChariots(0x02,defender_map);
	}
	
	packet_wrapper h1(128);
	using namespace S2C;
	CMD::Make<CMD::player_query_chariots>::From(h1,attacker_map.size(),defender_map.size());

	abase::hash_map<int, int>::iterator it = attacker_map.begin(), eit = attacker_map.end();
	for ( ; it != eit; ++it)
	{
		CMD::Make<CMD::player_query_chariots>::Add(h1, it->first, it->second);
	}

	it = defender_map.begin();
	eit = defender_map.end();
	for ( ; it != eit; ++it)
	{
		CMD::Make<CMD::player_query_chariots>::Add(h1, it->first, it->second);
	}

	send_ls_msg(GetParent(), h1);
}
