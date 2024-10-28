#include "clstab.h"
#include "faction.h"
#include "world.h"
#include "player_imp.h"
#include "instance/mnfaction_ctrl.h"
#include "invincible_filter.h"
#include "playermnfaction.h"

DEFINE_SUBSTANCE(gplayer_mnfaction,gplayer_imp,CLS_MNFACTION_PLAYER_IMP);

void gplayer_mnfaction::PlayerEnterWorld()
{
	gplayer_imp::PlayerEnterWorld();
}

void gplayer_mnfaction::PlayerEnterServer(int source_tag)
{
	SetBattleFaction();

	gplayer_imp::PlayerEnterServer(source_tag);

	mnfaction_ctrl* pCtrl = (mnfaction_ctrl *)_plane->w_ctrl;
	if(!GetFaction())
	{
		SetMnfactionDomainID(0);
		return;
	}
	_delay_start_timestamp = pCtrl -> PlayerSetBattleDelayStartime();
	SetMnfactionDomainID(pCtrl->_domain_id);
	SetExpireTime(pCtrl->_end_timestamp);
	
	int debuff_appear_time;
	float debuff_init_ratio;
	float debuff_enhance_ratio_per_minute;
	pCtrl -> GetDebuffInfo(debuff_appear_time, debuff_init_ratio, debuff_enhance_ratio_per_minute);
	SetDebuffInfo(debuff_appear_time, debuff_init_ratio, debuff_enhance_ratio_per_minute);

	int cur_time = g_timer.get_systime();
	if(cur_time < _expire_time && _expire_time - cur_time < _debuff_appear_time)//如果玩家在DEBUFF已经开始后进入，计算玩家身上应该有的DEBUFF
	{
		int start_add_debuff_time = _expire_time - _debuff_appear_time;
		_debuff_have_appear_time = (cur_time - start_add_debuff_time) / 60;
		_debuff_tick = DEBUFF_ENHANCE_INTERVAL;
	}

	_runner->mnfaction_player_faction_info(GetFaction(), _mnfaction_domain_id);

	int battle_ground_have_start_time = g_timer.get_systime() - (pCtrl -> _start_timestamp);
	_runner->mnfaction_battle_ground_have_start_time(battle_ground_have_start_time);

	EnableFreePVP(true);
}

void gplayer_mnfaction::SetDebuffInfo(int debuff_appear_time, float debuff_init_ratio, float debuff_enhance_ratio_per_minute)
{
	_debuff_appear_time              = debuff_appear_time;
	_debuff_init_ratio               = debuff_init_ratio;
	_debuff_enhance_ratio_per_minute = debuff_enhance_ratio_per_minute;
}

void gplayer_mnfaction::SetBattleFaction()
{
	gplayer * pPlayer = GetParent();
	if(pPlayer->IsBattleOffense())
	{
		//攻方
		_attack_faction = FACTION_BATTLEDEFENCE;
		_defense_faction = FACTION_BATTLEOFFENSE;
	}
	else if(pPlayer->IsBattleDefence())
	{
		//守方
		_attack_faction = FACTION_BATTLEOFFENSE;
		_defense_faction = FACTION_BATTLEDEFENCE;
	}
	else
	{
		_attack_faction = 0;
		_defense_faction = 0;
	}
}

void gplayer_mnfaction::OnDeath(const XID & lastattack,bool is_pariah, char attacker_mode, int taskdead)
{
	gplayer_imp::OnDeath(lastattack,is_pariah,attacker_mode,taskdead);

	gplayer * pPlayer = GetParent();
	if(pPlayer->IsBattleOffense() || pPlayer->IsBattleDefence())
	{
		if(_plane->w_ctrl) _plane->w_ctrl->OnPlayerDeath(pPlayer, lastattack, GetSoulPower(), _parent->pos);	
	}
}

void gplayer_mnfaction::PlayerLeaveServer()
{
	gplayer_imp::PlayerLeaveServer();
	
	mnfaction_ctrl * pCtrl = (mnfaction_ctrl *)_plane->w_ctrl;
	gplayer * pPlayer = GetParent();
	if(pPlayer->IsBattleOffense())
	{
		pCtrl->DelAttacker((gplayer*)_parent);
	}
	else if(pPlayer->IsBattleDefence())
	{
		pCtrl->DelDefender((gplayer*)_parent);
	}
	EnableFreePVP(false);
	pPlayer->ClrBattleMode();

	//如果玩家死亡则复活
	if(_parent->b_zombie)
	{
		_parent->b_zombie = false;
		_basic.hp = (int)(_cur_prop.max_hp * 0.1f + 0.5f);
		_basic.mp = (int)(_cur_prop.max_mp * 0.1f + 0.5f);

		SetRefreshState();
		_enemy_list.clear();

		((gplayer_controller*)_commander)->OnResurrect();

		//清除后继所有的session
		ClearNextSession();

		_runner->resurrect(0);
	}

	//清除所有的负面状态
	_filters.ClearSpecFilter(filter::FILTER_MASK_DEBUFF);
	_runner->mnfaction_player_faction_info(0, 0);
	GMSV::MNDomainBattleLeaveNotice(_parent->ID.id, GetMNFactionID(), GetMnfactionDomainID());
}

void gplayer_mnfaction::PlayerLeaveWorld()
{
	gplayer_imp::PlayerLeaveWorld();
	
	mnfaction_ctrl * pCtrl = (mnfaction_ctrl *)_plane->w_ctrl;
	gplayer * pPlayer = GetParent();
	if(pPlayer->IsBattleOffense())
	{
		pCtrl->DelAttacker((gplayer*)_parent);
	}
	else if(pPlayer->IsBattleDefence())
	{
		pCtrl->DelDefender((gplayer*)_parent);
	}
	EnableFreePVP(false);
	pPlayer->ClrBattleMode();

	//如果玩家死亡则复活
	if(_parent->b_zombie)
	{
		_parent->b_zombie = false;
		_basic.hp = (int)(_cur_prop.max_hp * 0.1f + 0.5f);
		_basic.mp = (int)(_cur_prop.max_mp * 0.1f + 0.5f);

		SetRefreshState();
		_enemy_list.clear();

		((gplayer_controller*)_commander)->OnResurrect();

		//清除后继所有的session
		ClearNextSession();

		_runner->resurrect(0);
	}

	//清除所有的负面状态
	_filters.ClearSpecFilter(filter::FILTER_MASK_DEBUFF);
	_runner->mnfaction_player_faction_info(0, 0);
	GMSV::MNDomainBattleLeaveNotice(_parent->ID.id, GetMNFactionID(), GetMnfactionDomainID());
}

int gplayer_mnfaction::GetFaction()
{
	return _defense_faction;
}

int gplayer_mnfaction::GetEnemyFaction()
{
	 return _attack_faction;
}

int gplayer_mnfaction::MessageHandler(world * pPlane ,const MSG & msg)
{
	switch(msg.message)
	{
	//攻击判定以帮派为最优
		case GM_MSG_ATTACK:
			{
				attack_msg ack_msg = *(attack_msg*)msg.content;
				_filters.EF_TransRecvAttack(msg.source, ack_msg);
				XID attacker = ack_msg.ainfo.attacker;

				//这里无需区分玩家和NPC
				//自己不允许攻击
				if(attacker.id == _parent->ID.id) return 0;
				if(!(ack_msg.target_faction  & (GetFaction())))
				{
					//派系不正确则返回
					return 0;
				}
				ack_msg.target_faction = 0xFFFFFFFF;
				ack_msg.is_invader =  false;

				//如果对方未开PK开关也不会被攻击
				if( attacker.IsPlayerClass())
				{
					if(!(ack_msg.attacker_mode & attack_msg::PVP_ENABLE))
					{
						return 0;
					}
				}

				//试着选择对象
				((gplayer_controller*)_commander)->TrySelect(msg.source);
				HandleAttackMsg(pPlane,msg,&ack_msg);
				return 0;
			}
			break;
		case GM_MSG_ENCHANT:
			{
				//进行处理
				enchant_msg ech_msg = *(enchant_msg*)msg.content;
				_filters.EF_TransRecvEnchant(msg.source, ech_msg);
				if(!ech_msg.helpful)
				{
					XID attacker = ech_msg.ainfo.attacker;
					//自己不允许攻击
					if(attacker.id == _parent->ID.id) return 0;
					if(!(ech_msg.target_faction  & (GetFaction())))
					{
						//派系不正确则返回
						return 0;
					}
					ech_msg.target_faction = 0xFFFFFFFF;
					
					//如果对方未开PK开关也不会被攻击
					if( attacker.IsPlayerClass())
					{
						if(!(ech_msg.attacker_mode & attack_msg::PVP_ENABLE))
						{
							return 0;
						}
					}

					((gplayer_controller*)_commander)->TrySelect(msg.source);
				}
				else
				{
					XID attacker = ech_msg.ainfo.attacker;
					if(attacker.id != _parent->ID.id)
					{
						//只有自己方可以使用有益法术
						if(!(ech_msg.attacker_faction  &  _defense_faction))
						{
							//派系不正确则返回
							return 0;
						}
						//玩家可以选择是否接受他人祝福
						if(ech_msg.helpful == 1)
						{
							if((_refuse_bless & C2S::REFUSE_NON_TEAMMATE_BLESS) && (!IsInTeam() || !IsMember(attacker))) return 0;
						}
						else if(ech_msg.helpful == 2)
						{
							if(_refuse_bless & C2S::REFUSE_NEUTRAL_BLESS) return 0;
						}
					}
				}
				ech_msg.is_invader = false;
				HandleEnchantMsg(pPlane,msg,&ech_msg);
				return 0;
			}
			break;
			/*case GM_MSG_TEAM_INVITE:
			{
				return 0;
			}
			break;
			case GM_MSG_TEAM_APPLY_PARTY:
			{
				return 0;
			}
			break;*/
	}
	return gplayer_imp::MessageHandler(pPlane,msg);
}

void gplayer_mnfaction::FillAttackMsg(const XID & target,attack_msg & attack,int dec_arrow)
{
	gplayer_imp::FillAttackMsg(target,attack,dec_arrow);
	attack.force_attack = 0;
	attack.attacker_faction |= _defense_faction;
	attack.target_faction |= _attack_faction;
}

void gplayer_mnfaction::FillEnchantMsg(const XID & target,enchant_msg & enchant)
{
	gplayer_imp::FillEnchantMsg(target,enchant);
	enchant.force_attack = 0;
	enchant.attacker_faction |= _defense_faction;
	enchant.target_faction |= _attack_faction;
}

gactive_imp::attack_judge gplayer_mnfaction::GetPetAttackHook()
{
	return gplayer_mnfaction::__GetPetAttackHook;
	
}

gactive_imp::enchant_judge gplayer_mnfaction::GetPetEnchantHook()
{
	return gplayer_mnfaction::__GetPetEnchantHook;
}

gactive_imp::attack_fill gplayer_mnfaction::GetPetAttackFill()
{
	return __GetPetAttackFill;
}

gactive_imp::enchant_fill gplayer_mnfaction::GetPetEnchantFill()
{
	return __GetPetEnchantFill;
}

bool gplayer_mnfaction::__GetPetAttackHook(gactive_imp * __this, const MSG & msg, attack_msg & amsg)
{
	//派系不正确则返回
	if(!(amsg.target_faction  & (__this->GetFaction()))) return false;
	amsg.target_faction = 0xFFFFFFFF;
	amsg.is_invader =  false;
	return true;
}

bool gplayer_mnfaction::__GetPetEnchantHook(gactive_imp * __this, const MSG & msg,enchant_msg & emsg)
{
	if(!emsg.helpful)
	{
		//派系不正确则返回
		if(!(emsg.target_faction  & (__this->GetFaction()))) return false;
		emsg.target_faction = 0xFFFFFFFF;
	}
	else
	{
		//只有对方不视自己是敌人才能够使用有益法术 这里和player_battle的并不完全一致
		if(emsg.target_faction  &  __this->GetFaction()) return false;
	}
	emsg.is_invader = false;
	return true;
}

void gplayer_mnfaction::__GetPetAttackFill(gactive_imp * __this, attack_msg & attack)
{
	gplayer_imp::__GetPetAttackFill(__this,attack);
	attack.force_attack = 0;
}

void gplayer_mnfaction::__GetPetEnchantFill(gactive_imp * __this, enchant_msg & enchant)
{
	gplayer_imp::__GetPetEnchantFill(__this,enchant);
	enchant.force_attack = 0;
}

void gplayer_mnfaction::OnHeartbeat(size_t tick)
{
	gplayer_imp::OnHeartbeat(tick);

	int cur_time = g_timer.get_systime();
	/*if(_expire_time && cur_time > _expire_time)
	{
		//GMSV::MNDomainBattleLeaveNotice(_parent->ID.id, unifid,GetMnfactionDomainID());
		//SetMnfactionDomainID(0);
		return;
	}*/
	
	if(cur_time < _expire_time && _expire_time - cur_time < _debuff_appear_time)
	{
		if(_debuff_have_appear_time == 0)
		{
			_skill.MnFactionAddFilter(object_interface(this), _debuff_init_ratio);
			++_debuff_have_appear_time;
			_debuff_tick = 0;
			_runner->mnfaction_shout_at_the_client(DEBUFF_APPEAR, 0);
		}
		else
		{
			++_debuff_tick;
			if(_debuff_tick >= DEBUFF_ENHANCE_INTERVAL)
			{
				_debuff_tick = 0;
				_skill.MnFactionAddFilter(object_interface(this), _debuff_init_ratio + _debuff_enhance_ratio_per_minute * _debuff_have_appear_time);
				++_debuff_have_appear_time;
			}
		}
	}
	
	mnfaction_ctrl * pCtrl = (mnfaction_ctrl *)_plane->w_ctrl;

	if((_delay_start_timestamp - cur_time) / 60 >= 0)
	{
		if((_delay_start_timestamp - cur_time) % 60 == 0)
		{
			_runner->mnfaction_shout_at_the_client(BATTLE_GROUND_FROM_START_TIME, (_delay_start_timestamp - cur_time) / 60 );
		}
	}
	
	if(++_sync_pos_tick >= SYNC_POS_INTERVAL)
	{
		_sync_pos_tick = 0;
		pCtrl -> PlayerPosInfoSync(_parent->ID.id, GetFaction(), _parent->pos);
		
	}

	int attacker_resource_point;
	int defender_resource_point;
	int attend_attacker_player_count;
	int attend_defender_player_count;
	int attacker_killed_player_count;
	int defender_killed_player_count;
	bool is_small_boss_appear;
	abase::vector<int> cur_degree;
	abase::vector<MNFactionStateInfo> attacker_resouse_tower_state;
	abase::vector<MNFactionStateInfo> defender_resouse_tower_state;
	abase::vector<MNFactionStateInfo> switch_tower_state;
	abase::vector<MNFactionStateInfo> transmit_pos_state;

		
	pCtrl->GetMnFactionInfo(attacker_resource_point, defender_resource_point, attend_attacker_player_count, attend_defender_player_count, cur_degree, attacker_resouse_tower_state, defender_resouse_tower_state, switch_tower_state, transmit_pos_state, attacker_killed_player_count, defender_killed_player_count, is_small_boss_appear);
	
	if(attacker_resource_point != _attacker_resource_point || defender_resource_point != _defender_resource_point)
	{
		_attacker_resource_point = attacker_resource_point;
		_defender_resource_point = defender_resource_point;
		_runner->mnfaction_resource_point_info(attacker_resource_point, defender_resource_point);
	}
	if(attend_attacker_player_count != _attend_attacker_player_count || attend_defender_player_count != _attend_defender_player_count)
	{
		_attend_attacker_player_count = attend_attacker_player_count;
		_attend_defender_player_count = attend_defender_player_count;
		_runner->mnfaction_player_count_info(attend_attacker_player_count, attend_defender_player_count);
	}
	if(attacker_killed_player_count != _attacker_killed_player_count || defender_killed_player_count != _defender_killed_player_count)
	{
		_attacker_killed_player_count = attacker_killed_player_count;
		_defender_killed_player_count = defender_killed_player_count;
		_runner->mnfaction_faction_killed_player_num(attacker_killed_player_count, defender_killed_player_count);
	}
	if(is_small_boss_appear != _is_small_boss_appear)
	{
		_is_small_boss_appear = is_small_boss_appear;
		_runner->mnfaction_shout_at_the_client(SMALL_BOSS_APPEAR, 0);
	}
	/*if(_debuff_have_appear_time == 1)
	{
	}*/
	for(size_t i = 0; i < cur_degree.size(); i++)
	{
		if(_cur_degree.size() < cur_degree.size())
		{
			_cur_degree.push_back(cur_degree[i]);
			_runner->mnfaction_resource_point_state_info(i, cur_degree[i]);	
		}
		else
		{
			if(_cur_degree[i] != cur_degree[i])
			{
				_cur_degree[i] = cur_degree[i];
				_runner->mnfaction_resource_point_state_info(i, cur_degree[i]);	
			}
		}
	}
	for(size_t i = 0 ; i < attacker_resouse_tower_state.size(); i++)
	{
		if(_attacker_resouse_tower_state.size() < attacker_resouse_tower_state.size())
		{
			_attacker_resouse_tower_state.push_back(attacker_resouse_tower_state[i]);
			_runner->mnfaction_resource_tower_state_info(1, attacker_resouse_tower_state[i]);
		
		}
		else
		{
			if(_attacker_resouse_tower_state[i]._state != attacker_resouse_tower_state[i]._state)
			{
				_attacker_resouse_tower_state[i] = attacker_resouse_tower_state[i];
				_runner->mnfaction_resource_tower_state_info(1, attacker_resouse_tower_state[i]);
			}
		}
	}
	for(size_t i = 0 ; i < defender_resouse_tower_state.size(); i++)
	{
		if(_defender_resouse_tower_state.size() < defender_resouse_tower_state.size())
		{
			_defender_resouse_tower_state.push_back(defender_resouse_tower_state[i]);
			_runner->mnfaction_resource_tower_state_info(1, defender_resouse_tower_state[i]);
		}
		else
		{
			if(_defender_resouse_tower_state[i]._state != defender_resouse_tower_state[i]._state)
			{
				_defender_resouse_tower_state[i] = defender_resouse_tower_state[i];
				_runner->mnfaction_resource_tower_state_info(1, defender_resouse_tower_state[i]);
			}
		}
	}
	for(size_t i = 0 ; i < switch_tower_state.size(); i++)
	{
		if(_switch_tower_state.size() < switch_tower_state.size())
		{
			_switch_tower_state.push_back(switch_tower_state[i]);
			_runner->mnfaction_switch_tower_state_info(1, switch_tower_state[i]);
		}
		else
		{
			if(_switch_tower_state[i]._state != switch_tower_state[i]._state)
			{
				_switch_tower_state[i] = switch_tower_state[i];
				_runner->mnfaction_switch_tower_state_info(1, switch_tower_state[i]);
			}
		}
	}
	for(size_t i = 0 ; i < transmit_pos_state.size(); i++)
	{
		if(_transmit_pos_state.size() < transmit_pos_state.size())
		{
			_transmit_pos_state.push_back(transmit_pos_state[i]);
			_runner->mnfaction_transmit_pos_state_info(1, transmit_pos_state[i]);
		}
		else
		{
			if(_transmit_pos_state[i]._state != transmit_pos_state[i]._state)
			{
				_transmit_pos_state[i] = transmit_pos_state[i];
				_runner->mnfaction_transmit_pos_state_info(1, transmit_pos_state[i]);
			}
		}
	}
}
