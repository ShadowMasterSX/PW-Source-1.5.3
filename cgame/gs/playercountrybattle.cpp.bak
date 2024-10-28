#include "clstab.h"
#include "faction.h"
#include "world.h"
#include "player_imp.h"
#include "playercountrybattle.h"
#include "instance/countrybattle_ctrl.h"
#include "invincible_filter.h"

DEFINE_SUBSTANCE(gplayer_countrybattle,gplayer_imp,CLS_COUNTRYBATTLE_PLAYER_IMP)

void 
gplayer_countrybattle::OnHeartbeat(size_t tick)
{
	if(_parent->IsZombie() && (g_timer.get_systime() & 0x03) == 0)
	{
		LazySendTo<0>(GM_MSG_GM_RESURRECT,_parent->ID,0,TICK_PER_SEC*3);
	}

	gplayer_imp::OnHeartbeat(tick);
	
	if(IsCombatState())	++ combat_time;
	++ attend_time;
	if(++ sync_counter >= 30)
	{
		sync_counter = 0;
		SyncScoreToPlane();
	}
	if(flag_carrier)
	{
		if(-- flag_timeout <= 0) 
		{
			SetFlagCarrier(false);
		}
		else
		{
			if(((gplayer*)_parent)->IsInvisible()) _filters.RemoveFilter(FILTER_INDEX_INVISIBLE);
			if(sync_counter % 2 == 0)
			{
				if(_plane->w_ctrl) _plane->w_ctrl->UpdateFlagCarrier(_parent->ID.id, _parent->pos);	
			}
			if(sync_counter % 3 == 0)
			{
				if(world_manager::GetInstance()->IsReachFlagGoal(((gplayer*)_parent)->IsBattleOffense(), _parent->pos))
				{
					if(_plane->w_ctrl) _plane->w_ctrl->HandInFlag(GetParent());
				}
			}
		}
	}
}

int 
gplayer_countrybattle::MessageHandler(world * pPlane ,const MSG & msg)
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
		case GM_MSG_COUNTRYBATTLE_HURT_RESULT:
			{
				ASSERT(msg.content_length == sizeof(int));
				int target_soulpower = *(int*)msg.content;
				if(target_soulpower)
				{
					damage_output +=  msg.param;
					bucket.output_weighted += (int)(0.0001f * msg.param * target_soulpower);
					if(bucket.output_weighted >= bucket.output_weighted_ceil)
					{
						bucket.output_weighted = bucket.output_weighted_ceil;
					}
				}
				else
				{
					damage_output_npc += msg.param;
				}
				return 0;
			}
		case GM_MSG_COUNTRYBATTLE_PLAYER_KILLED:
			{
				damage_outout_weighted = bucket.output_weighted;
				damage_endure = bucket.endure;
				return 0;
			}
	}
	return gplayer_imp::MessageHandler(pPlane,msg);
}

int 
gplayer_countrybattle::ZombieMessageHandler(world * pPlane ,const MSG & msg)
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
gplayer_countrybattle::FillAttackMsg(const XID & target,attack_msg & attack,int dec_arrow)
{
	gplayer_imp::FillAttackMsg(target,attack,dec_arrow);
	attack.force_attack = 0;
	attack.attacker_faction |= defense_faction;
	attack.target_faction |= attack_faction;
}

void
gplayer_countrybattle::FillEnchantMsg(const XID & target,enchant_msg & enchant)
{
	gplayer_imp::FillEnchantMsg(target,enchant);
	enchant.force_attack = 0;
	enchant.attacker_faction |= defense_faction;
	enchant.target_faction |= attack_faction;
}

void 
gplayer_countrybattle::PlayerEnterWorld()
{	
	gplayer_imp::PlayerEnterWorld();

	//正常情况下，这是不会出现的
	attack_faction = 0;
	defense_faction = 0;
	_runner->countrybattle_resurrect_rest_times(resurrect_rest_times);
}

void 
gplayer_countrybattle::PlayerEnterServer()
{
	//提前设置FACTION，因为playerenterserver函数会用到
	SetBattleFaction();

	gplayer_imp::PlayerEnterServer();
	gplayer * pPlayer = GetParent();

	countrybattle_ctrl * pCtrl = (countrybattle_ctrl *)_plane->w_ctrl;
	int country_player_total = 0;
	
	int damage_average = (_cur_prop.damage_low + _cur_prop.damage_high) / 2;
	int magic_damage_average = (_cur_prop.damage_magic_low + _cur_prop.damage_magic_high) / 2;
	int dmg_fac = (damage_average > magic_damage_average ? damage_average : magic_damage_average);
	bucket.output_weighted_ceil = (int)(0.0001f * dmg_fac * GetSoulPower());
	bucket.endure_ceil = (int)(3 * _cur_prop.max_hp);
	bucket.output_weighted = 0;
	bucket.endure = 0;
	
	//考虑完成攻方守方的判断
	if(pPlayer->IsBattleOffense())
	{
		//攻方
		//attack_faction = FACTION_BATTLEDEFENCE;
		//defense_faction = FACTION_BATTLEOFFENSE | FACTION_OFFENSE_FRIEND;
		_runner->enter_countrybattle(1,pCtrl->_data.battle_id,pCtrl->_data.end_timestamp,pCtrl->_data.country_attacker,pCtrl->_data.country_defender);
		pCtrl->PlayerEnter(pPlayer,0x01);
		_plane->ModifyCommonValue(COMMON_VALUE_ID_ATTACKER_COUNT, 1);
		country_player_total = pCtrl->_data.attacker_total;
	}
	else if(pPlayer->IsBattleDefence())
	{
		//守方
		//attack_faction = FACTION_BATTLEOFFENSE;
		//defense_faction = FACTION_BATTLEDEFENCE | FACTION_DEFENCE_FRIEND;
		_runner->enter_countrybattle(2,pCtrl->_data.battle_id,pCtrl->_data.end_timestamp,pCtrl->_data.country_attacker,pCtrl->_data.country_defender);
		pCtrl->PlayerEnter(pPlayer,0x02);
		_plane->ModifyCommonValue(COMMON_VALUE_ID_DEFENDER_COUNT, 1);
		country_player_total = pCtrl->_data.defender_total;
	}
	else
	{
		//attack_faction = 0;
		//defense_faction = 0;
		_runner->enter_countrybattle(0,pCtrl->_data.battle_id,pCtrl->_data.end_timestamp,pCtrl->_data.country_attacker,pCtrl->_data.country_defender);
		pCtrl->PlayerEnter(pPlayer,0x0);
	}
	EnableFreePVP(true);
	_runner->countrybattle_resurrect_rest_times(resurrect_rest_times);
	//加弱国保护buff
	if(country_player_total && country_player_total < pCtrl->_data.max_total && GetSoulPower() <= 30000)
	{
		float factor = 15000.f / GetSoulPower() * (pCtrl->_data.max_total - country_player_total) / country_player_total;
		int attack = (int)(30*factor);
		int defend = (int)(20*factor);
		if(attack > 15) attack = 15;
		if(defend > 10) defend = 10;
		_skill.CountryBattleWeakProtect(object_interface(this), 3600, attack, defend);	
	}
}

void 
gplayer_countrybattle::PlayerLeaveServer()
{
	gplayer_imp::PlayerLeaveServer();
	gplayer * pPlayer = GetParent();
	countrybattle_ctrl * pCtrl = (countrybattle_ctrl *)_plane->w_ctrl;

	SyncScoreToPlane();

	//进行攻守双方人员记数的减少
	//这个人数的增加是在battleground_world_message_handler的PlayerPreEnterServer里做的
	if(pPlayer->IsBattleOffense())
	{
		//攻方
		pCtrl->DelAttacker();
		pCtrl->PlayerLeave(pPlayer,0x01);
		_plane->ModifyCommonValue(COMMON_VALUE_ID_ATTACKER_COUNT, -1);

	}
	else if(pPlayer->IsBattleDefence())
	{
		//守方
		pCtrl->DelDefender();
		pCtrl->PlayerLeave(pPlayer,0x02);
		_plane->ModifyCommonValue(COMMON_VALUE_ID_DEFENDER_COUNT, -1);
	}
	else
	{
		pCtrl->PlayerLeave(pPlayer,0x0);	
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

	_runner->enter_countrybattle(0,0,0,0,0);
}

void 
gplayer_countrybattle::PlayerLeaveWorld()
{
	gplayer_imp::PlayerLeaveWorld();
	gplayer * pPlayer = GetParent();
	countrybattle_ctrl * pCtrl = (countrybattle_ctrl *)_plane->w_ctrl;

	SyncScoreToPlane();

	//进行攻守双方人员记数的减少
	//这个人数的增加是在battleground_world_message_handler的PlayerPreEnterServer里做的
	if(pPlayer->IsBattleOffense())
	{
		//攻方
		pCtrl->DelAttacker();
		pCtrl->PlayerLeave(pPlayer,0x01);
		_plane->ModifyCommonValue(COMMON_VALUE_ID_ATTACKER_COUNT, -1);

	}
	else if(pPlayer->IsBattleDefence())
	{
		//守方
		pCtrl->DelDefender();
		pCtrl->PlayerLeave(pPlayer,0x02);
		_plane->ModifyCommonValue(COMMON_VALUE_ID_DEFENDER_COUNT, -1);
	}
	else
	{
		pCtrl->PlayerLeave(pPlayer,0x0);	
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

	_runner->enter_countrybattle(0,0,0,0,0);
}

int 
gplayer_countrybattle::GetFaction()
{
	return _faction | defense_faction;
}

int 
gplayer_countrybattle::GetEnemyFaction()
{
	return _enemy_faction | attack_faction;
}

gactive_imp::attack_judge 
gplayer_countrybattle::GetPetAttackHook()
{
	return gplayer_countrybattle::__GetPetAttackHook;
	
}

gactive_imp::enchant_judge 
gplayer_countrybattle::GetPetEnchantHook()
{
	return gplayer_countrybattle::__GetPetEnchantHook;
}

gactive_imp::attack_fill 
gplayer_countrybattle::GetPetAttackFill()
{
	return __GetPetAttackFill;
}

gactive_imp::enchant_fill 
gplayer_countrybattle::GetPetEnchantFill()
{
	return __GetPetEnchantFill;
}

bool 
gplayer_countrybattle::__GetPetAttackHook(gactive_imp * __this, const MSG & msg, attack_msg & amsg)
{
	//派系不正确则返回
	if(!(amsg.target_faction  & (__this->GetFaction()))) return false;
	amsg.target_faction = 0xFFFFFFFF;
	amsg.is_invader =  false;
	return true;
}

bool 
gplayer_countrybattle::__GetPetEnchantHook(gactive_imp * __this, const MSG & msg,enchant_msg & emsg)
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
gplayer_countrybattle::__GetPetAttackFill(gactive_imp * __this, attack_msg & attack)
{
	gplayer_imp::__GetPetAttackFill(__this,attack);
	attack.force_attack = 0;
}

void
gplayer_countrybattle::__GetPetEnchantFill(gactive_imp * __this, enchant_msg & enchant)
{
	gplayer_imp::__GetPetEnchantFill(__this,enchant);
	enchant.force_attack = 0;
}

void 
gplayer_countrybattle::SetBattleFaction()
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

void 
gplayer_countrybattle::OnDamage(const XID & attacker,int skill_id,const attacker_info_t&info,int damage,int at_state,char speed,bool orange,unsigned char section)
{
	gplayer_imp::OnDamage(attacker,skill_id,info,damage,at_state,speed,orange,section);
	
	bucket.endure += damage;
	if(bucket.endure >= bucket.endure_ceil) bucket.endure = bucket.endure_ceil;
	
	if(attacker.IsPlayer())
	{
		int soulpower = GetSoulPower();
		SendTo<0>(GM_MSG_COUNTRYBATTLE_HURT_RESULT, attacker, damage, &soulpower, sizeof(int));
	}
}

void 
gplayer_countrybattle::OnHurt(const XID & attacker,const attacker_info_t&info,int damage,bool invader)
{
	gplayer_imp::OnHurt(attacker,info,damage,invader);
	bucket.endure += damage;	
	if(bucket.endure >= bucket.endure_ceil) bucket.endure = bucket.endure_ceil;
	if(attacker.IsPlayer())
	{
		int soulpower = GetSoulPower();
		SendTo<0>(GM_MSG_COUNTRYBATTLE_HURT_RESULT, attacker, damage, &soulpower, sizeof(int));
	}
}

void 
gplayer_countrybattle::OnDeath(const XID & lastattack,bool is_pariah, char attacker_mode, int taskdead)
{
	gplayer_imp::OnDeath(lastattack,is_pariah,attacker_mode,taskdead);

	gplayer * pPlayer = GetParent();
	if(pPlayer->IsBattleOffense() || pPlayer->IsBattleDefence())
	{
		if(_plane->w_ctrl) _plane->w_ctrl->OnPlayerDeath(pPlayer, lastattack, GetSoulPower(), _parent->pos);	
	}
	//死亡旗帜消失
	if(flag_carrier) SetFlagCarrier(false);

	damage_outout_weighted = bucket.output_weighted;
	damage_endure = bucket.endure;

	const float PLAYER_DEATH_NOTIFY_DISATNCE = 50.0f;
	MSG msg;
	BuildMessage(msg, GM_MSG_COUNTRYBATTLE_PLAYER_KILLED, XID(GM_TYPE_PLAYER,-1), _parent->ID, _parent->pos);
	_plane->BroadcastSphereMessage(msg, _parent->pos, PLAYER_DEATH_NOTIFY_DISATNCE);
}

bool 
gplayer_countrybattle::CanResurrect(int param)
{
	if(resurrect_rest_times <= 0) return false;

	return gplayer_imp::CanResurrect(param);
}

int
gplayer_countrybattle::Resurrect(const A3DVECTOR & pos,bool nomove,float exp_reduce,int target_tag,float hp_factor, float mp_factor, int param)
{
	gplayer_imp::Resurrect(pos,nomove,exp_reduce, target_tag,hp_factor,mp_factor,param);

	//设置无敌
	_filters.AddFilter(new invincible_filter(this,FILTER_INVINCIBLE,15));
	_skill.OnCountryBattleRevive(object_interface(this), 9, 150, 4.f, 4.f, 1800, 0.6f, 0.6f, 0.3f);

	-- resurrect_rest_times;
	_runner->countrybattle_resurrect_rest_times(resurrect_rest_times);
	return 0;
}

void 
gplayer_countrybattle::SyncScoreToPlane()
{
	gplayer * pPlayer = GetParent();
	if(pPlayer->IsBattleOffense()) 
	{
		if(_plane->w_ctrl)
		{
			_plane->w_ctrl->UpdatePersonalScore(true, _parent->ID.id, combat_time, attend_time, damage_output, damage_outout_weighted, damage_endure, damage_output_npc);
			combat_time = attend_time = damage_output = damage_outout_weighted = damage_endure = damage_output_npc = 0;
		}
	}
	else if(pPlayer->IsBattleDefence())
	{
		if(_plane->w_ctrl)
		{
			_plane->w_ctrl->UpdatePersonalScore(false, _parent->ID.id, combat_time, attend_time, damage_output, damage_outout_weighted, damage_endure, damage_output_npc);
			combat_time = attend_time = damage_output = damage_outout_weighted = damage_endure = damage_output_npc = 0;
		}
	}
}

void 
gplayer_countrybattle::SetFlagCarrier(bool b)
{ 
	if(b)
	{
		flag_carrier = true;
		flag_timeout = FLAG_TIMEOUT;
		_runner->countrybattle_became_flag_carrier(1);
		_skill.SetFlager(object_interface(this), 0.25f, -0.5f, -0.5f, 0.2f);
	}
	else
	{
		flag_carrier = false;
		flag_timeout = 0;
		_runner->countrybattle_became_flag_carrier(0);
		_skill.UnSetFlager(object_interface(this));
	}
}


