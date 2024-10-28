#include "clstab.h"
#include "faction.h"
#include "world.h"
#include "player_imp.h"
#include "playerbattlebase.h"

DEFINE_SUBSTANCE(gplayer_battle_base,gplayer_imp,CLS_BATTLE_BASE_PLAYER_IMP)

int 
gplayer_battle_base::MessageHandler(world * pPlane ,const MSG & msg)
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
					((gplayer_controller*)_commander)->TrySelect(msg.source);
				}
				else
				{
					XID attacker = ech_msg.ainfo.attacker;
					if(attacker.id != _parent->ID.id)
					{
						//只有自己方可以使用有益法术
						if(!(ech_msg.attacker_faction  &  defense_faction))
						{
							//派系不正确则返回
							return 0;
						}
						//玩家可以选择是否接受他人祝福
						if(ech_msg.helpful == 1)
						{
							if((_refuse_bless & C2S::REFUSE_NON_TEAMMATE_BLESS) 
									&& (!IsInTeam() || !IsMember(attacker))) return 0;
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
	}
	return gplayer_imp::MessageHandler(pPlane,msg);
}

int 
gplayer_battle_base::ZombieMessageHandler(world * pPlane ,const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_SCROLL_RESURRECT:
		{
			//考虑这个地方也要考虑派系
			EnterResurrectReadyState(0.0f,DEFAULT_RESURRECT_HP_FACTOR,DEFAULT_RESURRECT_MP_FACTOR);
		};
		return 0;

		case GM_MSG_ENCHANT_ZOMBIE:
		{
			ASSERT(msg.content_length >= sizeof(enchant_msg));
			enchant_msg ech_msg = *(enchant_msg*)msg.content;
			if(!ech_msg.helpful)
			{
				return 0;
			}
			else
			{
				XID attacker = ech_msg.ainfo.attacker;
				if(attacker.id != _parent->ID.id)
				{
					//只有自己方可以使用有益法术
					if(!(ech_msg.attacker_faction  &  defense_faction))
					{
						//派系不正确则返回
						return 0;
					}
					//玩家可以选择是否接受他人祝福
					if(ech_msg.helpful == 1)
					{
						if((_refuse_bless & C2S::REFUSE_NON_TEAMMATE_BLESS) 
								&& (!IsInTeam() || !IsMember(attacker))) return 0;
					}
					else if(ech_msg.helpful == 2)
					{
						if(_refuse_bless & C2S::REFUSE_NEUTRAL_BLESS) return 0;
					}
				}
			}
			ech_msg.is_invader = false;
			HandleEnchantMsg(pPlane,msg,&ech_msg);
		}
		return 0;

		default:
		return gplayer_imp::ZombieMessageHandler(pPlane, msg);
	}
	return 0;
}

void
gplayer_battle_base::FillAttackMsg(const XID & target,attack_msg & attack,int dec_arrow)
{
	gplayer_imp::FillAttackMsg(target,attack,dec_arrow);
	attack.force_attack = 0;
}

void
gplayer_battle_base::FillEnchantMsg(const XID & target,enchant_msg & enchant)
{
	gplayer_imp::FillEnchantMsg(target,enchant);
	enchant.force_attack = 0;
}

void 
gplayer_battle_base::PlayerEnterWorld()
{	
	gplayer_imp::PlayerEnterWorld();

	//正常情况下，这是不会出现的
	attack_faction = 0;
	defense_faction = 0;
}

void 
gplayer_battle_base::PlayerEnterServer()
{
	//提前设置FACTION，因为playerenterserver函数会用到
	SetBattleFaction();

	gplayer_imp::PlayerEnterServer();
	gplayer * pPlayer = GetParent();

	//考虑完成攻方守方的判断
	if(pPlayer->IsBattleOffense())
	{
		//攻方
		//attack_faction = FACTION_BATTLEDEFENCE;
		//defense_faction = FACTION_BATTLEOFFENSE | FACTION_OFFENSE_FRIEND;
		_plane->ModifyCommonValue(COMMON_VALUE_ID_ATTACKER_COUNT, 1);
	}
	else if(pPlayer->IsBattleDefence())
	{
		//守方
		//attack_faction = FACTION_BATTLEOFFENSE;
		//defense_faction = FACTION_BATTLEDEFENCE | FACTION_DEFENCE_FRIEND;
		_plane->ModifyCommonValue(COMMON_VALUE_ID_DEFENDER_COUNT, 1);
	}
	else
	{
		//attack_faction = 0;
		//defense_faction = 0;
	}
	EnableFreePVP(true);
}

void 
gplayer_battle_base::PlayerLeaveServer()
{
	gplayer_imp::PlayerLeaveServer();
	gplayer * pPlayer = GetParent();

	if(pPlayer->IsBattleOffense())
	{
		//攻方
		_plane->ModifyCommonValue(COMMON_VALUE_ID_ATTACKER_COUNT, -1);

	}
	else if(pPlayer->IsBattleDefence())
	{
		//守方
		_plane->ModifyCommonValue(COMMON_VALUE_ID_DEFENDER_COUNT, -1);
	}

	pPlayer->ClrBattleMode();
	EnableFreePVP(false);

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
}

void 
gplayer_battle_base::PlayerLeaveWorld()
{
	gplayer_imp::PlayerLeaveWorld();
	gplayer * pPlayer = GetParent();

	if(pPlayer->IsBattleOffense())
	{
		//攻方
		_plane->ModifyCommonValue(COMMON_VALUE_ID_ATTACKER_COUNT, -1);

	}
	else if(pPlayer->IsBattleDefence())
	{
		//守方
		_plane->ModifyCommonValue(COMMON_VALUE_ID_DEFENDER_COUNT, -1);
	}

	pPlayer->ClrBattleMode();
	EnableFreePVP(false);

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
}

int 
gplayer_battle_base::GetFaction()
{
	return _faction | defense_faction;
}

int 
gplayer_battle_base::GetEnemyFaction()
{
	return _enemy_faction | attack_faction;
}

gactive_imp::attack_judge 
gplayer_battle_base::GetPetAttackHook()
{
	return gplayer_battle_base::__GetPetAttackHook;
	
}

gactive_imp::enchant_judge 
gplayer_battle_base::GetPetEnchantHook()
{
	return gplayer_battle_base::__GetPetEnchantHook;
}

gactive_imp::attack_fill 
gplayer_battle_base::GetPetAttackFill()
{
	return __GetPetAttackFill;
}

gactive_imp::enchant_fill 
gplayer_battle_base::GetPetEnchantFill()
{
	return __GetPetEnchantFill;
}

bool 
gplayer_battle_base::__GetPetAttackHook(gactive_imp * __this, const MSG & msg, attack_msg & amsg)
{
	//派系不正确则返回
	if(!(amsg.target_faction  & (__this->GetFaction()))) return false;
	amsg.target_faction = 0xFFFFFFFF;
	amsg.is_invader =  false;
	return true;
}

bool 
gplayer_battle_base::__GetPetEnchantHook(gactive_imp * __this, const MSG & msg,enchant_msg & emsg)
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

void
gplayer_battle_base::__GetPetAttackFill(gactive_imp * __this, attack_msg & attack)
{
	gplayer_imp::__GetPetAttackFill(__this,attack);
	attack.force_attack = 0;
}

void
gplayer_battle_base::__GetPetEnchantFill(gactive_imp * __this, enchant_msg & enchant)
{
	gplayer_imp::__GetPetEnchantFill(__this,enchant);
	enchant.force_attack = 0;
}

void 
gplayer_battle_base::SetBattleFaction()
{
	gplayer * pPlayer = GetParent();
	if(pPlayer->IsBattleOffense())
	{
		//攻方
		attack_faction = FACTION_BATTLEDEFENCE;
		defense_faction = FACTION_BATTLEOFFENSE | FACTION_OFFENSE_FRIEND;
	}
	else if(pPlayer->IsBattleDefence())
	{
		//守方
		attack_faction = FACTION_BATTLEOFFENSE;
		defense_faction = FACTION_BATTLEDEFENCE | FACTION_DEFENCE_FRIEND;
	}
	else
	{
		attack_faction = 0;
		defense_faction = 0;
	}
}
