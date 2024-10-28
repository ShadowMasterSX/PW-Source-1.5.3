#include <stdlib.h>
#include "string.h"
#include "world.h"
#include <common/types.h>
#include <common/message.h>
#include "filter_man.h"
#include "actobject.h"
#include "actsession.h"
#include "playertemplate.h"
#include "obj_interface.h"
#include "player_imp.h"
#include "npc.h"
#include "item.h"
#include "invincible_filter.h"
#include "invisible_filter.h"
#include "npcgenerator.h"
#include "clstab.h"
#include "ainpc.h"
#include <gsp_if.h>
#include "petnpc.h"
#include "pathfinding/pathfinding.h"
#include "cooldowncfg.h"
#include "servicenpc.h"
#include "playerlimit.h"
#include "nonpenalty_pvp_filter.h"

void 
object_interface::BeHurt(const XID & who,const attacker_info_t & info, int damage,char attacker_mode)
{
	return BeHurt(who,info,damage,false,attacker_mode);
}

void 
object_interface::BeHurt(const XID & who,const attacker_info_t & info, int damage, bool invader,char attacker_mode)
{
	ASSERT(damage >= 0);
	_imp->BeHurt(who,info,damage,invader,attacker_mode);
}

void 
object_interface::SendHurtMsgToSelf(const XID & attacker, int damage, bool invader, char attack_mode)
{
	msg_hurt_extra_info_t data = {invader, attack_mode};
	MSG msg;
	BuildMessage(msg,GM_MSG_HURT,_imp->_parent->ID,attacker,_imp->_parent->pos,damage,&data,sizeof(data));
	if(attack_mode & attack_msg::PVP_DUEL)
	{
		msg.message = GM_MSG_DUEL_HURT;
	}
	_imp->_plane->PostLazyMessage(msg);
}
	
void 
object_interface::DecHP(int hp)
{
	ASSERT(hp >= 0);
	_imp->DecHP(hp);
}

void 
object_interface::DecSkillPoint(int sp)
{
	_imp->DecSkillPoint(sp);
}

void 
object_interface::Heal(const XID & healer,size_t life)
{
	_imp->Heal(healer,life);
}

void 
object_interface::Heal(size_t life)
{
	_imp->Heal(life);
}

void 
object_interface::HealBySkill(const XID & healer,size_t life)
{
	_imp->HealBySkill(healer,life);
}

void 
object_interface::HealBySkill(size_t life)
{
	_imp->HealBySkill(life);
}

void
object_interface::HealByPotion(size_t life)
{
	_imp->HealByPotion(life);
}

bool 
object_interface::DrainMana(int mana)
{
	return _imp->DrainMana(mana);
}

void
object_interface::InjectMana(int mana)
{
	_imp->InjectMana(mana);
}

//增强
void 
object_interface::EnhanceHPGen(int hpgen)
{
	_imp->_en_point.hp_gen += hpgen;
	_imp->_cur_prop.hp_gen += hpgen;
}

void 
object_interface::EnhanceScaleHPGen(int hpgen)
{
	_imp->_en_percent.hp_gen += hpgen;
}

void 
object_interface::ImpairScaleHPGen(int hpgen)
{
	_imp->_en_percent.hp_gen -= hpgen;
}

void 
object_interface::EnhanceScaleMPGen(int mpgen)
{
	_imp->_en_percent.mp_gen += mpgen;
}

void 
object_interface::ImpairScaleMPGen(int mpgen)
{
	_imp->_en_percent.mp_gen -= mpgen;
}

void 
object_interface::UpdateHPMPGen()
{
	property_policy::UpdatePlayerMPHPGen(_imp);
}

void 
object_interface::ImpairHPGen(int hpgen)
{
	_imp->_en_point.hp_gen -= hpgen;
	_imp->_cur_prop.hp_gen -= hpgen;
}


void 
object_interface::EnhanceMPGen(int mpgen)
{
	_imp->_en_point.mp_gen += mpgen;
	_imp->_cur_prop.mp_gen += mpgen;
}

void 
object_interface::ImpairMPGen(int mpgen)
{
	_imp->_en_point.mp_gen -= mpgen;
	_imp->_cur_prop.mp_gen -= mpgen;
}

void 
object_interface::EnhanceMaxHP(int hp)
{
	_imp->_en_point.max_hp += hp;
	property_policy::UpdateLife(_imp);
	_imp->SetRefreshState();
}

void 
object_interface::ImpairMaxHP(int hp)
{
	_imp->_en_point.max_hp -= hp;
	property_policy::UpdateLife(_imp);
	_imp->SetRefreshState();
}
void 
object_interface::EnhanceMaxMP(int mp)
{
	_imp->_en_point.max_mp += mp;
	property_policy::UpdateMana(_imp);
	_imp->SetRefreshState();
}

void 
object_interface::ImpairMaxMP(int mp)
{
	_imp->_en_point.max_mp -= mp;
	property_policy::UpdateMana(_imp);
	_imp->SetRefreshState();
}

void 
object_interface::EnhanceScaleMaxHP(int hp,bool update)
{
	_imp->_en_percent.max_hp += hp;
	if(update)
	{
	property_policy::UpdateLife(_imp);
	_imp->SetRefreshState();
	}
}
void 
object_interface::ImpairScaleMaxHP(int hp,bool update)
{
	_imp->_en_percent.max_hp -= hp;
	if(update)
	{
	property_policy::UpdateLife(_imp);
	_imp->SetRefreshState();
	}
}
void 
object_interface::EnhanceScaleMaxMP(int mp)
{
	_imp->_en_percent.max_mp += mp;
	property_policy::UpdateMana(_imp);
	_imp->SetRefreshState();
}

void 
object_interface::ImpairScaleMaxMP(int mp)
{
	_imp->_en_percent.max_mp -= mp;
	property_policy::UpdateMana(_imp);
	_imp->SetRefreshState();
}

void 
object_interface::EnhanceScaleExp(float exp_sp_factor, float realm_exp_factor)
{
    _imp->_exp_sp_factor += exp_sp_factor;
    _imp->_realm_exp_factor += realm_exp_factor;
}

void
object_interface::ImpairScaleExp(float exp_sp_factor, float realm_exp_factor)
{
    _imp->_exp_sp_factor -= exp_sp_factor;
    _imp->_realm_exp_factor -= realm_exp_factor;
}

void 
object_interface::EnhanceDefense(int def)
{
	_imp->_en_point.defense += def;
}

void 
object_interface::EnhanceScaleAttack(int attack)
{
	_imp->_en_percent.attack += attack;
}

void 
object_interface::ImpairScaleAttack(int attack)
{
	_imp->_en_percent.attack -= attack;
}

void 
object_interface::EnhanceAttackRange(float range)
{
	_imp->_en_point.attack_range += range;
}

void 
object_interface::ImpairAttackRange(float range)
{
	if((_imp->_en_point.attack_range -= range) < 1e-3) _imp->_en_point.attack_range = 0.f;
}


void 
object_interface::ImpairDefense(int def)
{
	_imp->_en_point.defense -= def;
}

void 
object_interface::EnhanceScaleDefense(int def)
{
	_imp->_en_percent.defense += def;
}

void 
object_interface::ImpairScaleDefense(int def)
{
	_imp->_en_percent.defense -= def;
}

void
object_interface::EnhanceArmor(int ac)
{
	_imp->_en_point.armor += ac;
}

void 
object_interface::ImpairArmor(int ac)
{
	_imp->_en_point.armor -= ac;
}

void 
object_interface::EnhanceScaleArmor(int ac)
{
	_imp->_en_percent.armor += ac;
}

void 
object_interface::ImpairScaleArmor(int ac)
{
	_imp->_en_percent.armor -= ac;
}

void 
object_interface::EnhanceResistance(size_t cls, int res)
{
	ASSERT(cls < MAGIC_CLASS);
	_imp->_en_point.resistance[cls] += res;
}

void 
object_interface::ImpairResistance(size_t cls, int res)
{
	ASSERT(cls < MAGIC_CLASS);
	_imp->_en_point.resistance[cls] -= res;
}


void 
object_interface::ImpairScaleResistance(size_t cls, int res)
{
	ASSERT(cls < MAGIC_CLASS);
	_imp->_en_percent.resistance[cls] -= res;
}

void 
object_interface::EnhanceScaleResistance(size_t cls, int res)
{
	ASSERT(cls < MAGIC_CLASS);
	_imp->_en_percent.resistance[cls] += res;
}

void 
object_interface::IncAntiDefenseDegree(int val)
{
    _imp->_anti_defense_degree += val;
    _imp->SetRefreshState();
}

void
object_interface::DecAntiDefenseDegree(int val)
{
    _imp->_anti_defense_degree -= val;
    _imp->SetRefreshState();
}

void
object_interface::IncAntiResistanceDegree(int val)
{
    _imp->_anti_resistance_degree += val;
    _imp->SetRefreshState();
}

void
object_interface::DecAntiResistanceDegree(int val)
{
    _imp->_anti_resistance_degree -= val;
    _imp->SetRefreshState();
}

void 
object_interface::EnhanceDamage(int dmg)
{
	_imp->_en_point.damage_low += dmg;
	_imp->_en_point.damage_high += dmg;
}

void 
object_interface::ImpairDamage(int dmg)
{
	_imp->_en_point.damage_low -= dmg;
	_imp->_en_point.damage_high -= dmg;
}

void 
object_interface::EnhanceMagicDamage(int dmg)
{
	_imp->_en_point.magic_dmg_low += dmg;
	_imp->_en_point.magic_dmg_high += dmg;
}

void 
object_interface::ImpairMagicDamage(int dmg)
{
	_imp->_en_point.magic_dmg_low -= dmg;
	_imp->_en_point.magic_dmg_high -= dmg;
}

void 
object_interface::EnhanceScaleDamage(int dmg)
{
	_imp->_en_percent.damage += dmg;
}

void 
object_interface::ImpairScaleDamage(int dmg)
{
	_imp->_en_percent.damage -= dmg;
}

void 
object_interface::EnhanceScaleMagicDamage(int dmg)
{
	_imp->_en_percent.magic_dmg += dmg;
}

void 
object_interface::ImpairScaleMagicDamage(int dmg)
{
	_imp->_en_percent.magic_dmg -= dmg;
}

void 
object_interface::EnhanceSpeed(int speedup)
{
	_imp->_en_percent.walk_speed += speedup;
	_imp->_en_percent.run_speed  += speedup;
}

void 
object_interface::ImpairSpeed(int speedup)
{
	_imp->_en_percent.walk_speed -= speedup;
	_imp->_en_percent.run_speed  -= speedup;
}

void 
object_interface::EnhanceSpeed0(float speedup)
{
	_imp->_en_point.walk_speed += speedup;
	_imp->_en_point.run_speed  += speedup;
}

void 
object_interface::ImpairSpeed0(float speedup)
{
	_imp->_en_point.walk_speed -= speedup;
	_imp->_en_point.run_speed  -= speedup;
}

void 
object_interface::EnhanceOverrideSpeed(float speed)
{
	_imp->_en_point.override_speed += speed;
}

void 
object_interface::ImpairOverrideSpeed(float speed)
{
	if((_imp->_en_point.override_speed -= speed) <= 1e-3)
	{
		_imp->_en_point.override_speed  = 0;
	}
}

void 
object_interface::EnhanceSwimSpeed(int speedup)
{
	_imp->_en_percent.swim_speed  += speedup;
}

void 
object_interface::ImpairSwimSpeed(int speedup)
{
	_imp->_en_percent.swim_speed  -= speedup;
}

void 
object_interface::EnhanceFlySpeed(float speedup)
{	
	_imp->_en_point.flight_speed += speedup;
}

void 
object_interface::ImpairFlySpeed(float speedup)
{
	_imp->_en_point.flight_speed -= speedup;
}

void 
object_interface::EnhanceScaleFlySpeed(int speedup)
{	
	_imp->_en_percent.flight_speed += speedup;
}

void 
object_interface::ImpairScaleFlySpeed(int speedup)
{
	_imp->_en_percent.flight_speed -= speedup;
}

void 
object_interface::EnhanceMountSpeed(int speedup)
{	
	_imp->_en_percent.mount_speed += speedup;
}

void 
object_interface::ImpairMountSpeed(int speedup)
{
	_imp->_en_percent.mount_speed -= speedup;
}

void 
object_interface::InjectBreath(int value)
{
	_imp->InjectBreath(value);
}

void 
object_interface::EnableEndlessBreath(bool bVal)
{
	_imp->EnableEndlessBreath(bVal);
}

void 
object_interface::AdjustBreathDecPoint(int offset)
{
	_imp->AdjustBreathDecPoint(offset);
}

void 
object_interface::EnhanceAttackSpeed(int speedup)
{
	_imp->_en_point.attack_speed  -= speedup;
}

void 
object_interface::ImpairAttackSpeed(int speedup)
{
	_imp->_en_point.attack_speed  += speedup;
}

void 
object_interface::EnhanceScaleAttackSpeed(int speedup)
{
	_imp->_en_percent.attack_speed -= speedup;
}

void 
object_interface::ImpairScaleAttackSpeed(int speedup)
{
	_imp->_en_percent.attack_speed += speedup;
}

void 
object_interface::EnhanceAttackDegree(int value)
{
	_imp->_attack_degree += value;
}

void 
object_interface::ImpairAttackDegree(int value)
{
	_imp->_attack_degree -= value;
}

void 
object_interface::EnhanceDefendDegree(int value)
{
	_imp->_defend_degree += value;
}

void 
object_interface::ImpairDefendDegree(int value)
{
	_imp->_defend_degree -= value;
}


//属性
const basic_prop & 
object_interface::GetBasicProp()
{
	return _imp->_basic;
}

const extend_prop & 
object_interface::GetExtendProp()
{
	return _imp->_cur_prop;
}

const enhanced_param & 
object_interface::GetEnhancedParam()
{
	return _imp->_en_point;
}

const scale_enhanced_param & 	
object_interface::GetScaleEnhanecdParam()
{
	return _imp->_en_percent;
}

const item_prop &
object_interface::GetCurWeapon()
{
	return _imp->_cur_item;
}

int object_interface::GetDefendDegree()
{
	return _imp->_defend_degree;	
}
int object_interface::GetAttackDegree()
{
	return _imp->_attack_degree;
}

int object_interface::GetCrit()
{
	return _imp->_crit_rate + _imp->_base_crit_rate;
}

int object_interface::GetCritDamage()
{
	return _imp->_crit_damage_bonus + CRIT_DAMAGE_BONUS_PERCENT;
}

int object_interface::GetSoulPower()
{
	return _imp->GetSoulPower();
}

int object_interface::GetInvisibleDegree()
{
	return ((gactive_object*)_imp->_parent)->invisible_degree;	
}


int object_interface::GetAntiInvisibleDegree()
{
	return ((gactive_object*)_imp->_parent)->anti_invisible_degree;	
}

int object_interface::GetReputation()
{
	return _imp->OI_GetReputation();
}

int object_interface::GetSkillEnhance()
{
	return _imp->_skill_enhance;
}

int object_interface::GetSkillEnhance2()
{
	return _imp->_skill_enhance2;
}

int object_interface::GetHistoricalMaxLevel()
{
	return _imp->GetHistoricalMaxLevel();
}

int object_interface::GetVigour()
{
	return _imp->GetVigour();
}

int object_interface::GetRealm()
{
	return _imp->OI_GetRealm();
}

int object_interface::GetLocalVal(int index)
{
	return _imp->GetLocalVal(index);
}

//filter
void 
object_interface::AddFilter(filter* fp)
{
	ASSERT(fp);
	_imp->_filters.AddFilter(fp);
}

void 
object_interface::RemoveFilter(int filter_id)
{
	_imp->_filters.RemoveFilter(filter_id);
}

bool
object_interface::Resurrect(float exp_reduce, float hp_factor, float mp_factor)
{
	//return _imp->Resurrect(exp_reduce);
	_imp->EnterResurrectReadyState(exp_reduce,hp_factor,mp_factor);
	return true;
}

//查询 
bool
object_interface::IsDead()
{
	return _imp->_parent->IsZombie();
}

bool 
object_interface::IsMember(const XID & member)
{
	return _imp->OI_IsMember(member);
}

bool
object_interface::IsInTeam()
{
	return _imp->OI_IsInTeam();
}

bool
object_interface::IsTeamLeader()
{
	return _imp->OI_IsTeamLeader();
}

bool
object_interface::IsPalsyImmune()
{
	if((IsPlayerClass() || IsPet()) && GetImmuneMask()&(0x00140000))	// 同 statedef.h
		return true; // 玩家走新的免疫
	else if(!IsPlayerClass() && !IsPet() && GetImmuneMask()&(0x00040040)) // 怪物走老的免疫
		return true;
	return false;
}

bool 
object_interface::IsPVPEnable()
{
	return _imp->OI_IsPVPEnable();
}

char
object_interface::GetForceAttack()
{
	return _imp->OI_GetForceAttack();	
}

int 
object_interface::GetInvaderState()
{
	return _imp->OI_GetInvaderState();
}

int
object_interface::GetFaction()
{
	return _imp->GetFaction();
}

float 
object_interface::GetHpScale()
{
	gactive_object * pObj = (gactive_object*)_imp->_parent;
	return pObj->base_info.hp*1.f/pObj->base_info.max_hp;
}

bool 
object_interface::IsMafiaMember()
{
	return _imp->OI_IsMafiaMember();
}

int
object_interface::GetMafiaID()
{
	return _imp->OI_GetMafiaID();
}

char
object_interface::GetMafiaRank()
{
	return _imp->OI_GetMafiaRank();
}

int 
object_interface::GetSpouseID()
{
	return _imp->OI_GetSpouseID();
}

int
object_interface::GetClass()
{
	return _imp->GetObjectClass();
}

int
object_interface::GetEnemyFaction()
{
	return _imp->GetEnemyFaction();
}

int 
object_interface::GetWorldTag()
{
	return world_manager::GetWorldTag();
}

const A3DVECTOR &
object_interface::GetPos()
{
	return _imp->_parent->pos;
}

const XID &
object_interface::GetSelfID()
{
	return _imp->_parent->ID;
}

const XID &
object_interface::GetCurTargetID()
{
	return _imp->GetCurTarget();
}

const XID & 
object_interface::GetLeaderID()
{
	return _imp->GetLeaderID();
}

float
object_interface::GetBodySize()
{
	return _imp->_parent->body_size;
}

int
object_interface::QueryObject(const XID & who, A3DVECTOR & pos ,float & body_size)
{
	world::object_info info;
	if(!_imp->_plane->QueryObject(who,info)) return 0;
	pos = info.pos;
	body_size = info.body_size;
	if(info.state &  world::QUERY_OBJECT_STATE_ZOMBIE) return 2;
	return 1;
}

int 
object_interface::QueryObject(const XID & who, A3DVECTOR & pos, int & hp, int & mp)
{
	world::object_info info;
	if(!_imp->_plane->QueryObject(who,info)) return 0;
	pos = info.pos;
	hp = info.hp;
	mp = info.mp;
	if(info.state &  world::QUERY_OBJECT_STATE_ZOMBIE) return 2;
	return 1;
}

int 
object_interface::QueryObject(const XID & who, int & invisible_degree, int & anti_invisible_degree)
{
	world::object_info info;
	if(!_imp->_plane->QueryObject(who,info)) return 0;
	invisible_degree = info.invisible_degree;
	anti_invisible_degree = info.anti_invisible_degree;
	if(info.state &  world::QUERY_OBJECT_STATE_ZOMBIE) return 2;
	return 1;
}

int 
object_interface::GetTargetClass(const XID & target)
{
	if(target.IsPet()) return 258;
	else if(!target.IsPlayer()) return 127;
	world::object_info info;
	if(!_imp->_plane->QueryObject(target,info)) return -1;
	return info.race & 0x7FFFFFFF; 
}

int 
object_interface::GetTargetHp(const XID & target)
{
	world::object_info info;
	if(!_imp->_plane->QueryObject(target,info)) return -1;
	return info.hp; 
}

int 
object_interface::GetTargetMp(const XID & target)
{
	if(!target.IsPlayer()) return -1;
	world::object_info info;
	if(!_imp->_plane->QueryObject(target,info)) return -1;
	return info.mp; 
}

int 
object_interface::GetTargetMaxhp(const XID & target)
{
	world::object_info info;
	if(!_imp->_plane->QueryObject(target,info)) return -1;
	return info.max_hp; 
}

int 
object_interface::GetTargetLevel(const XID & target)
{
	world::object_info info;
	if(!_imp->_plane->QueryObject(target,info)) return -1;
	return info.level; 
}

void
object_interface::SendClientMsgSkillCasting(const XID &target, int skill, unsigned short time,unsigned char level)
{
	_imp->_runner->cast_skill(target,skill,time,level);
}

void
object_interface::SendClientMsgRuneSkillCasting(const XID &target, int skill, unsigned short time,unsigned char level)
{
	_imp->_runner->cast_rune_skill(target,skill,time,level);
}

void
object_interface::SendClientMsgSkillInterrupted(char reason)
{
	_imp->_runner->skill_interrupt(reason);
}

void 
object_interface::SendClientRushMode(unsigned char is_active)
{
	_imp->_runner->active_rush_mode(is_active);
}

void 
object_interface::SendClientSkillAbility(int id, int ability)
{
	_imp->_runner->skill_ability_notify(id, ability);
}

void 
object_interface::SendClientInstantSkill(const XID & target, int skill, unsigned char level)
{
	_imp->_runner->cast_instant_skill(target,skill,level);
}

void 
object_interface::SendClientRuneInstantSkill(const XID & target, int skill, unsigned char level)
{
	_imp->_runner->cast_rune_instant_skill(target,skill,level);
}

void 
object_interface::SendClientElfSkill(const XID & target, int skill, unsigned char level)   //lgc
{
	_imp->_runner->cast_elf_skill(target,skill,level);
}

void 
object_interface::SendClientCastPosSkill(const A3DVECTOR & pos, const XID &target, int skill,unsigned short time, unsigned char level)
{
	_imp->_runner->cast_pos_skill(pos,target,skill,time,level);
}

void
object_interface::SendClientMsgSkillPerform()
{
	_imp->_runner->skill_perform();
}

void 
object_interface::SendClientCurSpeed()
{
	_imp->_runner->get_extprop_move();
}

bool 
object_interface::CanAttack(const XID & target)
{
	return _imp->CanAttack(target);
}

int 
object_interface::GenerateDamage()
{
	int damage = _imp->GenerateAttackDamage();
	return damage;
}

int 
object_interface::GenerateMagicDamage()
{
	return _imp->GenerateMagicDamage();
}

int 
object_interface::GenerateDamage(int scale_enhance, int point_enhance)
{
	return _imp->GeneratePhysicDamage(scale_enhance,point_enhance);
}

int 
object_interface::GenerateMagicDamage(int scale_enhance, int point_enhance)
{
	return _imp->GenerateMaigicDamage2(scale_enhance,point_enhance);
}

int 
object_interface::GenerateDamageWithoutRune()
{
	int damage = _imp->GenerateAttackDamageWithoutRune();
	return damage;
}

int 
object_interface::GenerateMagicDamageWithoutRune()
{
	return _imp->GenerateMagicDamageWithoutRune();
}

int 
object_interface::GenerateDamageWithoutRune(int scale_enhance, int point_enhance)
{
	return _imp->GeneratePhysicDamageWithoutRune(scale_enhance,point_enhance);
}

int 
object_interface::GenerateMagicDamageWithoutRune(int scale_enhance, int point_enhance)
{
	return _imp->GenerateMaigicDamage2WithoutRune(scale_enhance,point_enhance);
}

int 
object_interface::GenerateEquipDamage()
{
	return _imp->GenerateEquipPhysicDamage();
}

int 
object_interface::GenerateEquipMagicDamage()
{
	return _imp->GenerateEquipMagicDamage();
}

//攻击
void object_interface::Attack(const XID & target, attack_msg & attack, int use_projectile)
{
	_imp->FillAttackMsg(target,attack,use_projectile);
	_imp->SendAttackMsg(target,attack);
}

void object_interface::RegionAttack1(const A3DVECTOR& pos, float radius,attack_msg & attack,int use_projectile,const XID& target)
{
	//球形
	//填充一下必要的数据
	_imp->FillAttackMsg(target,attack,use_projectile);

	//生成消息
	gobject *pObj = _imp->_parent;
	MSG msg;
	BuildMessage(msg,GM_MSG_ATTACK,XID(GM_TYPE_ACTIVE,-1),pObj->ID,pObj->pos,
	                  0,&attack,sizeof(attack));

	_imp->TranslateAttack(target,attack);
	//进行广播活动
	_imp->_plane->BroadcastSphereMessage(msg,pos,radius);
}

void object_interface::RegionAttack2(const A3DVECTOR& pos, float radius,attack_msg & attack,int use_projectile)
{
	//圆柱
	//填充一下必要的数据
	_imp->FillAttackMsg(XID(-1,-1),attack,use_projectile);

	//生成消息
	gobject *pObj = _imp->_parent;
	MSG msg;
	BuildMessage(msg,GM_MSG_ATTACK,XID(GM_TYPE_ACTIVE,-1),pObj->ID,pObj->pos,
	                  0,&attack,sizeof(attack));
	_imp->TranslateAttack(XID(-1,-1),attack);

	//进行广播活动 
	A3DVECTOR offset= pos;
	offset -= msg.pos;
	offset.normalize();
	offset *= attack.attack_range;
	offset += msg.pos;

	_imp->_plane->BroadcastCylinderMessage(msg,offset,radius);
}

void object_interface::RegionAttack3(const A3DVECTOR& pos, float cos_half_angle,attack_msg & attack,int use_projectile)
{
	//圆锥
	//填充一下必要的数据
	_imp->FillAttackMsg(XID(-1,-1),attack,use_projectile);

	//生成消息
	gobject *pObj = _imp->_parent;
	MSG msg;
	BuildMessage(msg,GM_MSG_ATTACK,XID(GM_TYPE_ACTIVE,-1),pObj->ID,pObj->pos,
	                  0,&attack,sizeof(attack));
	_imp->TranslateAttack(XID(-1,-1),attack);

	//进行广播活动
	_imp->_plane->BroadcastTaperMessage(msg,pos,attack.attack_range,cos_half_angle);
}

void object_interface::MultiAttack(const XID * target,size_t size, attack_msg & attack,int use_projectile)
{
	//多播
	//填充一下必要的数据
	_imp->FillAttackMsg(XID(-1,-1),attack,use_projectile);

	//生成消息
	gobject *pObj = _imp->_parent;
	MSG msg;
	BuildMessage(msg,GM_MSG_ATTACK,XID(-1,-1),pObj->ID,pObj->pos,
	                  0,&attack,sizeof(attack));
	_imp->TranslateAttack(XID(-1,-1),attack);

	//进行广播活动
	_imp->_plane->SendMessage(target, target + size, msg);
}

void object_interface::MultiAttack(const XID * target,size_t size, attack_msg & attack,MsgAdjust & adj,int use_projectile)
{
	//多播
	//填充一下必要的数据
	_imp->FillAttackMsg(*target,attack,use_projectile);

	//生成消息
	gobject *pObj = _imp->_parent;
	MSG msg;
	BuildMessage(msg,GM_MSG_ATTACK,XID(-1,-1),pObj->ID,pObj->pos,
	                  0,&attack,sizeof(attack));
	_imp->TranslateAttack(XID(-1,-1),attack);
	
	for(size_t i = 0; i < size; i ++)
	{
		msg.target = target[i];
		_imp->_plane->PostLazyMessage(msg);
		adj.AdjustAttack(attack);
	}
}

void object_interface::DelayAttack(const XID & target, attack_msg & attack, int use_projectile, size_t delay_tick)
{
	_imp->FillAttackMsg(target,attack,use_projectile);
	_imp->SendDelayAttackMsg(target,attack,delay_tick);
}

//技能
void object_interface::Enchant(const XID & target, enchant_msg & msg)
{
	_imp->FillEnchantMsg(target,msg);
	_imp->TranslateEnchant(target,msg);
	_imp->SendEnchantMsg(GM_MSG_ENCHANT,target,msg);
}

void object_interface::EnchantZombie(const XID & target, enchant_msg & msg)
{
	_imp->FillEnchantMsg(target,msg);
	_imp->TranslateEnchant(target,msg);
	_imp->SendEnchantMsg(GM_MSG_ENCHANT_ZOMBIE,target,msg);
}

void object_interface::RegionEnchant1(const A3DVECTOR& pos, float radius,enchant_msg & enchant, const XID& target)
{
	//球形
	//填充一下必要的数据
	_imp->FillEnchantMsg(target,enchant);

	//生成消息
	gobject *pObj = _imp->_parent;
	MSG msg;
	BuildMessage(msg,GM_MSG_ENCHANT,XID(GM_TYPE_ACTIVE,-1),pObj->ID,pObj->pos,
	                  0,&enchant,sizeof(enchant));

	_imp->TranslateEnchant(target,enchant);
	//进行广播活动
	_imp->_plane->BroadcastSphereMessage(msg,pos,radius);
}

void object_interface::RegionEnchant2(const A3DVECTOR& pos, float radius,enchant_msg & enchant )
{
	//圆柱
	//填充一下必要的数据
	_imp->FillEnchantMsg(XID(-1,-1),enchant);

	//生成消息
	gobject *pObj = _imp->_parent;
	MSG msg;
	BuildMessage(msg,GM_MSG_ENCHANT,XID(GM_TYPE_ACTIVE,-1),pObj->ID,pObj->pos,
	                  0,&enchant,sizeof(enchant));

	//进行广播活动
	A3DVECTOR offset= pos;
	offset -= msg.pos;
	offset.normalize();
	offset *= enchant.attack_range;
	offset += msg.pos;

	_imp->TranslateEnchant(XID(-1,-1),enchant);

	_imp->_plane->BroadcastCylinderMessage(msg,offset,radius);
}

void object_interface::RegionEnchant3(const A3DVECTOR& pos, float cos_half_angle,enchant_msg & enchant )
{
	//圆球
	//填充一下必要的数据
	_imp->FillEnchantMsg(XID(-1,-1),enchant);

	//生成消息
	gobject *pObj = _imp->_parent;
	MSG msg;
	BuildMessage(msg,GM_MSG_ENCHANT,XID(GM_TYPE_ACTIVE,-1),pObj->ID,pObj->pos,
	                  0,&enchant,sizeof(enchant));

	//进行广播活动
	_imp->TranslateEnchant(XID(-1,-1),enchant);
	_imp->_plane->BroadcastTaperMessage(msg,pos,enchant.attack_range,cos_half_angle);
}

void object_interface::MultiEnchant(const XID * target,size_t size, enchant_msg & enchant)
{
	//多播
	//填充一下必要的数据
	_imp->FillEnchantMsg(XID(-1,-1),enchant);

	//生成消息
	gobject *pObj = _imp->_parent;
	MSG msg;
	BuildMessage(msg,GM_MSG_ENCHANT,XID(-1,-1),pObj->ID,pObj->pos,
	                  0,&enchant,sizeof(enchant));

	//进行广播活动
	_imp->TranslateEnchant(XID(-1,-1),enchant);
	_imp->_plane->SendMessage(target, target + size, msg);
}

void object_interface::TeamEnchant(enchant_msg & enchant, bool exclude_self)
{
	ASSERT(IsInTeam());
	//多播
	//填充一下必要的数据
	_imp->FillEnchantMsg(XID(-1,-1),enchant);

	//生成消息
	gobject *pObj = _imp->_parent;
	MSG msg;
	BuildMessage(msg,GM_MSG_ENCHANT,XID(-1,-1),pObj->ID,pObj->pos,
	                  0,&enchant,sizeof(enchant));

	_imp->TranslateEnchant(XID(-1,-1),enchant);
	//进行广播活动
	_imp->SendMsgToTeam(msg,enchant.attack_range,exclude_self);
}

void object_interface::DelayEnchant(const XID & target, enchant_msg & msg, size_t delay_tick)
{
	_imp->FillEnchantMsg(target,msg);
	_imp->TranslateEnchant(target,msg);
	_imp->SendDelayEnchantMsg(GM_MSG_ENCHANT,target,msg,delay_tick);
}

//物品
int object_interface::TakeOutItem(int item_id)
{
	return _imp->TakeOutItem(item_id);
}

bool object_interface::CheckItem(int item_id,size_t count)
{
	return _imp->CheckItemExist(item_id,count);
}

bool object_interface::CheckItem(int inv_index, int item_id, size_t count)
{
	return _imp->CheckItemExist(inv_index,item_id,count);
}

size_t object_interface::QueryItemPrice(int inv_index,int item_id)
{
	return _imp->CheckItemPrice(inv_index, item_id);
}

size_t 
object_interface::QueryItemPrice(int item_id)
{
	return world_manager::GetDataMan().get_item_sell_price(item_id);
}

size_t object_interface::GetMoney()
{
	return _imp->GetMoneyAmount();
}

bool object_interface::UseProjectile(size_t count)
{
	return _imp->UseProjectile(count);
}

int object_interface::SpendFlyTime(int tick)
{
	return _imp->SpendFlyTime(tick);
}

int object_interface::GetFlyTime()
{
	return _imp->GetFlyTime();
}

void object_interface::TakeOff()
{
	_imp->TakeOff();
}

void object_interface::Landing()
{
	_imp->Landing();
}

bool object_interface::IsOnGround()
{
	return _imp->_layer_ctrl.GetLayer() == LAYER_GROUND;
}

bool object_interface::IsInAir()
{
	return _imp->_layer_ctrl.GetLayer() == LAYER_AIR;
}

bool object_interface::IsInWater()
{
	return _imp->_layer_ctrl.GetLayer() == LAYER_WATER;
}

bool object_interface::IsMountMode()
{
	return ((gactive_object*)_imp->_parent)->IsMountMode();
}

bool object_interface::IsInSanctuary(const A3DVECTOR & pos)
{
	return player_template::IsInSanctuary(pos);
}

void object_interface::SetNoFly(bool b)
{
	if(b && _imp->_filters.IsFilterExist(FILTER_FLY_EFFECT))
	{
		_imp->_filters.RemoveFilter(FILTER_FLY_EFFECT);
	}
	_imp->SetPlayerLimit(PLAYER_LIMIT_NOFLY, b);
}

void object_interface::SetNoLongJump(bool b)//禁止跳转
{
	_imp->SetPlayerLimit(PLAYER_LIMIT_NOLONGJUMP, b);
}

void object_interface::SetNoSpeedUp(bool b)//禁止各种百分比加速效果
{
	_imp->SetPlayerLimit(PLAYER_LIMIT_NOSPEEDUP, b);
}

void object_interface::SetNoInvisible(bool b)//禁止隐身
{
	if(b && _imp->_filters.IsFilterExist(FILTER_INDEX_INVISIBLE))
	{
		_imp->_filters.RemoveFilter(FILTER_INDEX_INVISIBLE);
	}
	_imp->SetPlayerLimit(PLAYER_LIMIT_NOINVISIBLE, b);
}

void object_interface::SetNoChangeSelect(bool b)
{
	_imp->SetPlayerLimit(PLAYER_LIMIT_NOCHANGESELECT, b);
}

void object_interface::SetNoMount(bool b)
{
	if(b && _imp->_filters.IsFilterExist(FILTER_INDEX_MOUNT_FILTER))
	{
		_imp->OI_RecallPet();
	}
	_imp->SetPlayerLimit(PLAYER_LIMIT_NOMOUNT, b);
}

void object_interface::SetNoAmulet(bool b)
{
	_imp->SetPlayerLimit(PLAYER_LIMIT_NOAMULET, b);
}

void object_interface::SetNoBind(bool b)
{
	if(b) _imp->OI_TryCancelPlayerBind();
	_imp->SetPlayerLimit(PLAYER_LIMIT_NOBIND, b);
}

bool object_interface::GetNoInvisible()
{
    return _imp->GetPlayerLimit(PLAYER_LIMIT_NOINVISIBLE);
}

void object_interface::DenyAttackCmd()
{
	_imp->DenyAttackCmd();
}

void object_interface::AllowAttackCmd()
{
	_imp->AllowAttackCmd();
}

void object_interface::DenyElfSkillCmd()
{
	_imp->DenyElfSkillCmd();
}

void object_interface::AllowElfSkillCmd()
{
	_imp->AllowElfSkillCmd();
}

void object_interface::DenyUseItemCmd()
{
	_imp->DenyUseItemCmd();
}

void object_interface::AllowUseItemCmd()
{
	_imp->AllowUseItemCmd();
}

void object_interface::DenyNormalAttackCmd()
{
	_imp->DenyNormalAttackCmd();
}

void object_interface::AllowNormalAttackCmd()
{
	_imp->AllowNormalAttackCmd();
}

void object_interface::DenyPetCmd()
{
	_imp->DenyPetCmd();
}

void object_interface::AllowPetCmd()
{
	_imp->AllowPetCmd();
}

bool object_interface::SessionOnAttacked(int session_id)
{
	act_session * pSession;
	if(_imp->GetCurSessionID() != session_id || !(pSession = _imp->GetCurSession()))
	{
		return true;
	}
	return pSession->OnAttacked();
}

bool object_interface::ActionOnAttacked(int action_id)
{
	return _imp->ActionOnAttacked(action_id);
}

void object_interface::SendClientMsgFlySwordTime(int time)
{
	_imp->_runner->flysword_time_capacity(gplayer_imp::IL_EQUIPMENT,item::EQUIP_INDEX_FLYSWORD,time);
}

void object_interface::IncVisibleState(unsigned short state)
{
	ASSERT(state < MAX_VISIBLE_STATE_COUNT);
	_imp->IncVisibleState(state);
}

void object_interface::DecVisibleState(unsigned short state)
{
	ASSERT(state < MAX_VISIBLE_STATE_COUNT);
	_imp->DecVisibleState(state);
}

void object_interface::ClearVisibleState(unsigned short state)
{
	ASSERT(state < MAX_VISIBLE_STATE_COUNT);
	_imp->ClearVisibleState(state);
}

void object_interface::InsertTeamVisibleState(unsigned short state)
{
	_imp->InsertTeamVisibleState(state, NULL, 0);
}

void object_interface::RemoveTeamVisibleState(unsigned short state)
{
	_imp->RemoveTeamVisibleState(state);
}

void object_interface::InsertTeamVisibleState(unsigned short state, int param)
{
	int p = g_timer.get_systime() + param;
	_imp->InsertTeamVisibleState(state, &p, 1);
}

void object_interface::InsertTeamVisibleState(unsigned short state, int param, int param2)
{
	int p[2] = {param, param2};
	_imp->InsertTeamVisibleState(state, p, 2);
}

void object_interface::InsertTeamVisibleState(unsigned short state, int param, int param2, int param3)
{
	int p[3] = {param, param2, param3};
	_imp->InsertTeamVisibleState(state, p, 3);
}

void object_interface::ModifyTeamVisibleState(unsigned short state, int param)
{
	_imp->ModifyTeamVisibleState(state, &param, 1);
}

void object_interface::ModifyTeamVisibleState(unsigned short state, int param, int param2)
{
	int p[2] = {param, param2};
	_imp->ModifyTeamVisibleState(state, p, 2);
}

void object_interface::ModifyTeamVisibleState(unsigned short state, int param, int param2, int param3)
{
	int p[3] = {param, param2, param3};
	_imp->ModifyTeamVisibleState(state, p, 3);
}

void object_interface::ChangeShape(int shape)
{
	_imp->ChangeShape(shape);
	_imp->_runner->change_shape(shape);
}

int object_interface::GetForm()
{
	return _imp->GetForm();
}

/*不用了
void object_interface::SetIdleMode(bool sleep,bool stun)
{
	_imp->SetIdleMode(sleep,stun);
}

void object_interface::GetIdleMode(bool & sleep, bool & stun)
{
	_imp->GetIdleMode(sleep,stun);
}


void object_interface::SetSealMode(bool silent,bool root)
{
	_imp->SetSealMode(silent,root);
}

void object_interface::GetSealMode(bool & silent, bool & root)
{
	_imp->GetSealMode(silent,root);
}*/

void object_interface::IncIdleSealMode(unsigned char mode)//lgc
{
	ASSERT(mode < 4);
	_imp->IncIdleSealMode(mode);	
}

void object_interface::DecIdleSealMode(unsigned char mode)//lgc
{
	ASSERT(mode < 4);
	_imp->DecIdleSealMode(mode);	
}

bool object_interface::IsAggressive()
{
	return _imp->_parent->msg_mask & gobject::MSG_MASK_PLAYER_MOVE;
}

void object_interface::SetAggressive(bool isActive)
{
	if(isActive)
	{
		_imp->_parent->msg_mask |= gobject::MSG_MASK_PLAYER_MOVE;
	}
	else
	{
		_imp->_parent->msg_mask &= ~gobject::MSG_MASK_PLAYER_MOVE;
	}
}

void object_interface::UpdateDefenseData()
{
	property_policy::UpdateDefense(_imp->GetObjectClass(),_imp);
}

void object_interface::UpdateAttackData()
{
	property_policy::UpdateAttack(_imp->GetObjectClass(),_imp);
}

void object_interface::UpdateMagicData()
{
	property_policy::UpdateMagic(_imp);
}

void object_interface::UpdateSpeedData()
{
	property_policy::UpdateSpeed(_imp);
}

void object_interface::SetAttackMonster()
{
	_imp->SetAttackMonster(true);
}

void object_interface::EnterCombatState()
{
	_imp->SetCombatState();
}

bool object_interface::IsCombatState()
{
	return _imp->IsCombatState();
}

void object_interface::AddAggro(const XID & who, int rage)
{
	if(rage <0) return;
	_imp->AddNPCAggro(who,rage);
}

void object_interface::AddAggroToEnemy(const XID & helper, int rage)
{
	if(rage <0) return;
	_imp->AddAggroToEnemy(helper,rage);
}

void object_interface::RemoveAggro(const XID& src, const XID& dest, float ratio)
{
    if (ratio <= 0) return;
    _imp->RemoveNPCAggro(src, dest, ratio);
}

void object_interface::KnockBack(const XID & attacker, const A3DVECTOR &source,float distance,int time,int stun_time)
{
	_imp->KnockBack(attacker,source, distance,time,stun_time);
}

void object_interface::PullOver(const XID & attacker, const A3DVECTOR &source,float distance, int time)
{
	_imp->PullOver(attacker, source, distance, time);
}

void object_interface::KnockUp(float distance,int time)
{
	_imp->KnockUp(distance,time);
}

void object_interface::SendClientNotifyRoot(unsigned char type)
{
	_imp->_runner->notify_root(type);
}

void object_interface::SendClientDispelRoot(unsigned char type)
{
	_imp->_runner->dispel_root(type);
}

void object_interface::SetATDefenseState(char state)
{
	_imp->__at_defense_state |= state;
}

GNET::SkillWrapper & object_interface::GetSkillWrapper()
{
	return _imp->_skill;
}
void 
object_interface::SetRetortState()
{
	_imp->SetRetortState();
}

bool
object_interface::GetRetortState()
{
	return _imp->GetRetortState();
}

void 
object_interface::SetAuraAttackState()
{
	_imp->SetAuraAttackState();
}

bool 
object_interface::GetAuraAttackState()
{
	return _imp->GetAuraAttackState();
}

void 
object_interface::SetReboundState()
{
	_imp->SetReboundState();
}

void 
object_interface::SetBeatBackState()
{
	_imp->SetBeatBackState();
}

void 
object_interface::SendHealMsg(const XID & target, int hp)
{
	_imp->SendTo<0>(GM_MSG_EXTERN_HEAL,target,hp);
}

void
object_interface::Reclaim()
{
	_imp->SendTo<0>(GM_MSG_OBJ_ZOMBIE_END,_imp->_parent->ID,0);
}

void
object_interface::SendClientEnchantResult(const XID & caster, int skill, char level , bool invader,int at_state,unsigned char section)
{
	_imp->_runner->enchant_result(caster, skill,level, invader,at_state, section);
}

void 
object_interface::SetInvincibleFilter(bool is_vin,int timeout,bool immune)
{
	if(is_vin)
	{
		AddFilter(new invincible_filter(_imp,FILTER_INVINCIBLE,timeout,immune));
	}
	else
	{
		RemoveFilter(FILTER_INVINCIBLE);
	}
}

void 
object_interface::SetInvincibleFilter2(bool is_vin,int timeout,bool immune)
{
	if(is_vin)
	{
		AddFilter(new invincible_filter_2(_imp,FILTER_INVINCIBLE,timeout,immune));
	}
	else
	{
		RemoveFilter(FILTER_INVINCIBLE);
	}
}

void 
object_interface::LockEquipment(bool is_lock)
{
	_imp->LockEquipment(is_lock);
}

void 
object_interface::BindToGound(bool is_bind)
{
	_imp->BindToGound(is_bind);
}

void
object_interface::DecMoney(size_t money)
{
	return _imp->DecMoneyAmount(money);
}

void 
object_interface::DropMoney(size_t money, bool isProtected)
{
	return _imp->DropMoneyAmount(money,isProtected);
}

void 
object_interface::ClearSpecFilter(int mask)
{
	_imp->_filters.ClearSpecFilter(mask);
}

bool 
object_interface::IsFilterExist(int id)
{
	return _imp->_filters.IsFilterExist(id);
}

int 
object_interface::QueryFilter(int filterid,int index)
{
	return _imp->_filters.QueryFilter(filterid, index);
}

void 
object_interface::BeTaunted(const XID & who, int addon_rage)
{
	_imp->BeTaunted(who,addon_rage);
}

int 
object_interface::CreateMinors(const A3DVECTOR & pos, const  object_interface::minor_param & param)
{
	npc_template * pTemplate = npc_stubs_manager::Get(param.mob_id);
	if(!pTemplate) return -1;
	if(pTemplate->mine_info.is_mine || pTemplate->npc_data.is_npc) return -1;
	
	npc_spawner::entry_t ent;
	memset(&ent,0,sizeof(ent));
	ent.npc_tid = param.mob_id;
	if(param.use_parent_faction)
	{
		//只有使用parent的faction时才会设置，否则是默认值 
		ent.accept_ask_for_help = true;
		ent.monster_faction_accept_for_help = _imp->GetMonsterFaction();
	}
	ent.mobs_count = 1;
	ent.msg_mask_or = 0;
	ent.msg_mask_and = 0xFFFFFFFF;
	ent.path_id = param.path_id;
	
	int aggro_policy = param.policy_aggro;
	int pcid = param.policy_classid;
	if(pcid == 0)
	{
		pcid = CLS_NPC_AI_POLICY_MINOR;
	}

	const int cid[3] = {CLS_NPC_IMP,CLS_NPC_DISPATCHER,CLS_NPC_CONTROLLER};
	gnpc * pNPC = npc_spawner::CreateMobBase(NULL,_imp->_plane,ent,-1,pos,cid,abase::Rand(0,255),pcid,aggro_policy,NULL);
	if(pNPC == NULL) 
		return -1;
	else
	{
		gnpc_imp * pImp = (gnpc_imp*)pNPC->imp;
		if(param.parent_is_leader)
		{
			pImp->_leader_id = _imp->_parent->ID;
		}
		else
		{
			pImp->_leader_id = param.spec_leader_id;
		}
		if(param.target_id.IsActive())
		{
			pImp->_target_id = param.target_id;
		}
		if(pTemplate->set_owner)
		{
			if (0 != param.owner_id.id && param.owner_id.IsPlayerClass())
				pImp->_owner_id = param.owner_id;
			else if(_imp->_parent->ID.IsPlayerClass())	//设置召唤者为怪物所有者
				pImp->_owner_id = _imp->_parent->ID;
			pImp->_record_dps_rank = pTemplate->record_dps_rank;
		}

		if(param.use_parent_faction)
		{
			pImp->_faction = _imp->_faction;
			pImp->_enemy_faction = _imp->GetEnemyFaction();
			pNPC->monster_faction |= _imp->GetFactionAskHelp();
		}
		pImp->_basic.exp =(int)(pImp->_basic.exp*param.exp_factor);
		if(pImp->_basic.exp < 0) pImp->_basic.exp = 0;

		pImp->_basic.skill_point =(int)(pImp->_basic.skill_point*param.sp_factor);
		if(pImp->_basic.skill_point < 0) pImp->_basic.skill_point = 0;

		pImp->_money_scale = param.money_scale;
		pImp->_drop_rate = param.drop_rate;
		
		pImp->SetLifeTime(param.remain_time);
		//pImp->SetDieWithLeader(param.die_with_leader);
		pImp->SetDieWithWho(param.die_with_who);

		if(param.vis_id)
		{
			pNPC->vis_tid = param.vis_id;
		}

		if(param.mob_name_size)
		{
			ASSERT(param.mob_name_size <= sizeof(param.mob_name)); 
			memcpy(pNPC->npc_name, param.mob_name,param.mob_name_size);
			pNPC->name_size = param.mob_name_size;
			pNPC->object_state |= gactive_object::STATE_NPC_NAME;
		}

		_imp->_plane->InsertNPC(pNPC);
		pImp->OnNpcEnterWorld();
		pImp->_runner->enter_world();
		pNPC->Unlock();
	}
	return pNPC->ID.id;
}

void 
object_interface::CreateMob(world *pPlane,const A3DVECTOR & pos, const  object_interface::minor_param & param)
{
	npc_template * pTemplate = npc_stubs_manager::Get(param.mob_id);
	if(!pTemplate) return;
	if(pTemplate->mine_info.is_mine || pTemplate->npc_data.is_npc) return;
	
	npc_spawner::entry_t ent;
	memset(&ent,0,sizeof(ent));
	ent.npc_tid = param.mob_id;
	ent.mobs_count = 1;
	ent.msg_mask_or = 0;
	ent.msg_mask_and = 0xFFFFFFFF;

	int aggro_policy = param.policy_aggro;
	int pcid = param.policy_classid;
	if(pcid == 0)
	{
		pcid = CLS_NPC_AI_POLICY_MINOR;
	}
	const int cid[3] = {CLS_NPC_IMP,CLS_NPC_DISPATCHER,CLS_NPC_CONTROLLER};
	gnpc * pNPC = npc_spawner::CreateMobBase(NULL,pPlane,ent,-1,pos,cid,abase::Rand(0,255),pcid,aggro_policy,NULL);
	if(pNPC)
	{
		gnpc_imp * pImp = (gnpc_imp*)pNPC->imp;

		//调整经验参数等
		pImp->_basic.exp =(int)(pImp->_basic.exp*param.exp_factor);
		if(pImp->_basic.exp < 0) pImp->_basic.exp = 0;

		pImp->_basic.skill_point =(int)(pImp->_basic.skill_point*param.sp_factor);
		if(pImp->_basic.skill_point < 0) pImp->_basic.skill_point = 0;

		pImp->_money_scale = param.money_scale;
		pImp->_drop_rate = param.drop_rate;
		
		pImp->SetLifeTime(param.remain_time);
		if(param.vis_id)
		{
			pNPC->vis_tid = param.vis_id;
		}

		if(param.mob_name_size)
		{
			ASSERT(param.mob_name_size <= sizeof(param.mob_name)); 
			memcpy(pNPC->npc_name, param.mob_name,param.mob_name_size);
			pNPC->name_size = param.mob_name_size;
			pNPC->object_state |= gactive_object::STATE_NPC_NAME;
		}
		
		if(pTemplate->set_owner && param.owner_id.IsPlayerClass())
		{
			//设置召唤者为怪物所有者
			pImp->_owner_id = param.owner_id;
			pImp->_record_dps_rank = pTemplate->record_dps_rank;
		}

		pPlane->InsertNPC(pNPC);
		pImp->OnNpcEnterWorld();
		pImp->_runner->enter_world();
		pNPC->Unlock();
	}
}

int
object_interface::CreateMinors(const  object_interface::minor_param & param,float radius)
{
	A3DVECTOR pos = _imp->_parent->pos;
	pos.x += abase::Rand(-radius,radius);
	pos.z += abase::Rand(-radius,radius);
	float height = _imp->_plane->GetHeightAt(pos.x,pos.z);
	if(pos.y < height) pos.y = height;

	return CreateMinors(pos,param);
}

int 
object_interface::CreateMinors(const  object_interface::minor_param & param,const XID & target,float radius)
{
	if(!target.IsActive()) return -1;
	world::object_info info;
	if(!_imp->_plane->QueryObject(target,info)) return -1;
	if(info.state & world::QUERY_OBJECT_STATE_ZOMBIE) return -1;
	if(!_imp->_plane->PosInWorld(info.pos)) return -1;
	
	//根据npc类型确定生成地点		
	npc_template * pTemplate = npc_stubs_manager::Get(param.mob_id);
	if(!pTemplate) return -1;
		
	int inhabit_env = 0;	//可以栖息的环境 bit0地bit1空bit2水
	switch(pTemplate->inhabit_type)
	{
		case 0:	//地面
			inhabit_env = 0x01;
			break;
		case 1: //水下
			inhabit_env = 0x04;
			break;
		case 2: //空中
			inhabit_env = 0x02;
			break;
		case 3: //地面加水下
			inhabit_env = 0x05;
			break;
		case 4: //地面加空中
			inhabit_env = 0x03;
			break;
		case 5: //水下加空中
			inhabit_env = 0x06;
			break;
		case 6: //海陆空
			inhabit_env = 0x07;
			break;
		default:
			inhabit_env = 0x01;
			break;
	}

	A3DVECTOR pos;
	int i;
	for(i=0; i<10; i++)
	{
		pos = info.pos;
		pos.x += abase::Rand(-radius,radius);
		pos.z += abase::Rand(-radius,radius);
		float height = _imp->_plane->GetHeightAt(pos.x,pos.z);
		float waterheight = path_finding::GetWaterHeight(_imp->_plane, pos.x,pos.z);

		int env_chk_seq[3] = {0x01, 0x02, 0x04};
		if(pos.y > height + 1.5f && pos.y > waterheight + 1.5)
		{
			env_chk_seq[0] = 0x02;
			env_chk_seq[1] = 0x01;
		}
		if(pos.y > height + 1.0f && pos.y < waterheight - 1.0f)
		{
			env_chk_seq[0] = 0x04;
			env_chk_seq[2] = 0x01;
		}
		
		int j;
		for(j=0; j<3; j++)
		{
			if(!(inhabit_env & env_chk_seq[j])) continue;
			if(env_chk_seq[j] & 0x01)	//地上
			{
				pos.y = height;
				if(path_finding::GetValidPos(_imp->_plane, pos)) break;
			}
			else if(env_chk_seq[j] & 0x02)	//空中
			{
				pos.y = info.pos.y + 2.0f;
				if(pos.y < height + 1.5) pos.y = height + 1.5f;
				if(pos.y < waterheight + 1.5f) pos.y = waterheight + 1.5f;
				if(path_finding::IsValidSPPos(_imp->_plane, pos)) break;
			}
			else if(env_chk_seq[j] & 0x04)	//水中
			{
				pos.y = info.pos.y;
				if(pos.y < height + 1.0f) pos.y = height + 1.0f;
				if(pos.y > waterheight - 1.0f) pos.y = waterheight - 1.0f;
				if(pos.y >= height + 1.0f && path_finding::IsValidSPPos(_imp->_plane, pos)) break;
			}
		}
		if(j<3) return CreateMinors(pos,param);
	}
	return -1;
}

int
object_interface::CreateMine(const A3DVECTOR & pos , const mine_param & param, const XID& target)
{
	npc_template * pTemplate = npc_stubs_manager::Get(param.mine_id);
	if(!pTemplate) return -1;
	if(!pTemplate->mine_info.is_mine) return -1;
	
	mine_spawner::entry_t ent;
	memset(&ent,0,sizeof(ent));
	ent.mid = param.mine_id;
	ent.mine_count = 1;
	ent.reborn_time = 10000;

	gmatter * pMatter = mine_spawner::CreateMine(NULL,pos, _imp->_plane,0,ent);
	if(pMatter == NULL) return -1;
	
	gmatter_mine_imp * pImp = (gmatter_mine_imp*)pMatter->imp;
	if (pImp == NULL) return -1;

	if(pTemplate->mine_info.set_owner)
	{
		//指定所有者
		if(target.IsPlayerClass())
			pImp->SetOwner(target);
		else if(_imp->_parent->ID.IsPlayerClass())
			pImp->SetOwner(_imp->_parent->ID);
	}

	//设定召唤出来的矿物的寿命
	pImp->SetLife(param.remain_time);

	pMatter->Unlock();
	return 0;
}

int 
object_interface::CreateMine(const A3DVECTOR & pos , const mine_param & param, const int dir, const XID& target)
{
	npc_template * pTemplate = npc_stubs_manager::Get(param.mine_id);
	if(!pTemplate) return -1;
	if(!pTemplate->mine_info.is_mine) return -1;
	
	mine_spawner::entry_t ent;
	memset(&ent,0,sizeof(ent));
	ent.mid = param.mine_id;
	ent.mine_count = 1;
	ent.reborn_time = 10000;

	gmatter * pMatter = mine_spawner::CreateMine2(NULL,pos, _imp->_plane,0,ent,0,0,dir);
	if(pMatter == NULL) return -1;
	
	gmatter_mine_imp * pImp = (gmatter_mine_imp*)pMatter->imp;
	if (pImp == NULL) return -1;

	if(pTemplate->mine_info.set_owner)
	{
		//指定所有者
		if(target.IsPlayerClass())
			pImp->SetOwner(target);
		else if(_imp->_parent->ID.IsPlayerClass())
			pImp->SetOwner(_imp->_parent->ID);
	}

	//设定召唤出来的矿物的寿命
	pImp->SetLife(param.remain_time);

	pMatter->Unlock();
	return 0;
}

int 
object_interface::CreateMine(const mine_param & param, const XID & target, float radius)
{
	if(!target.IsActive()) return -1;
	world::object_info info;
	if(!_imp->_plane->QueryObject(target,info)) return -1;
	if(info.state & world::QUERY_OBJECT_STATE_ZOMBIE) return -1;
	if(!_imp->_plane->PosInWorld(info.pos)) return -1;
	
	A3DVECTOR pos;
	int i;
	for(i=0; i<10; i++)
	{
		//在有怪物通过图的位置才成生成矿
		pos = info.pos;
		pos.x += abase::Rand(-radius,radius);
		pos.z += abase::Rand(-radius,radius);
		pos.y = _imp->_plane->GetHeightAt(pos.x,pos.z);
		if(!path_finding::GetValidPos(_imp->_plane, pos)) continue;
		
		return  CreateMine(pos,param, param.bind_target ? target:XID(-1,-1));	
	}
	return -1;
}

int
object_interface::CreateNPC(const A3DVECTOR & pos, const npc_param & param)
{
	npc_template * pTemplate = npc_stubs_manager::Get(param.npc_id);
	if(!pTemplate) return -1; 
	if(!pTemplate->npc_data.is_npc) return -1;

	npc_spawner::entry_t ent;
	memset(&ent,0,sizeof(ent));
	ent.npc_tid = param.npc_id;
	ent.path_id = param.path_id;
	ent.mobs_count = 1;
	ent.msg_mask_or = 0;
	ent.msg_mask_and = 0xFFFFFFFF;
	
	int cid[3]={-1,-1,-1};
	gnpc * pNPC = npc_spawner::CreateNPCBase(NULL,_imp->_plane,ent,-1,pos,cid,abase::Rand(0,255));
	if(!pNPC) return -1;

	gnpc_imp * pImp = (gnpc_imp*)pNPC->imp;
	pImp->SetLifeTime(param.remain_time);

	pNPC->imp->_plane->InsertNPC(pNPC);
	pNPC->imp->OnNpcEnterWorld();
	pNPC->imp->_runner->enter_world();
	pNPC->Unlock();
	
	return 0;
}

int 
object_interface::CreateNPC(const npc_param & param, const XID & target, float radius)
{
	if(!target.IsActive()) return -1;
	world::object_info info;
	if(!_imp->_plane->QueryObject(target,info)) return -1;
	if(info.state & world::QUERY_OBJECT_STATE_ZOMBIE) return -1;
	if(!_imp->_plane->PosInWorld(info.pos)) return -1;
	
	//根据npc类型确定生成地点		
	npc_template * pTemplate = npc_stubs_manager::Get(param.npc_id);
	if(!pTemplate) return -1;
	
	int inhabit_env = 0;	//可以栖息的环境 bit0地bit1空bit2水
	switch(pTemplate->inhabit_type)
	{
		case 0:	//地面
			inhabit_env = 0x01;
			break;
		case 1: //水下
			inhabit_env = 0x04;
			break;
		case 2: //空中
			inhabit_env = 0x02;
			break;
		case 3: //地面加水下
			inhabit_env = 0x05;
			break;
		case 4: //地面加空中
			inhabit_env = 0x03;
			break;
		case 5: //水下加空中
			inhabit_env = 0x06;
			break;
		case 6: //海陆空
			inhabit_env = 0x07;
			break;
		default:
			inhabit_env = 0x01;
			break;
	}

	A3DVECTOR pos;
	int i;
	for(i=0; i<10; i++)
	{
		pos = info.pos;
		pos.x += abase::Rand(-radius,radius);
		pos.z += abase::Rand(-radius,radius);
		float height = _imp->_plane->GetHeightAt(pos.x,pos.z);
		float waterheight = path_finding::GetWaterHeight(_imp->_plane, pos.x,pos.z);
	
		int env_chk_seq[3] = {0x01, 0x02, 0x04};
		if(pos.y > height + 1.5f && pos.y > waterheight + 1.5)
		{
			env_chk_seq[0] = 0x02;
			env_chk_seq[1] = 0x01;
		}
		if(pos.y > height + 1.0f && pos.y < waterheight - 1.0f)
		{
			env_chk_seq[0] = 0x04;
			env_chk_seq[2] = 0x01;
		}
		
		int j;
		for(j=0; j<3; j++)
		{
			if(!(inhabit_env & env_chk_seq[j])) continue;
			if(env_chk_seq[j] & 0x01)	//地上
			{
				pos.y = height;
				if(path_finding::GetValidPos(_imp->_plane, pos)) break;
			}
			else if(env_chk_seq[j] & 0x02)	//空中
			{
				pos.y = info.pos.y;
				if(pos.y < height + 1.5) pos.y = height + 1.5f;
				if(pos.y < waterheight + 1.5f) pos.y = waterheight + 1.5f;
				if(path_finding::IsValidSPPos(_imp->_plane, pos)) break;
			}
			else if(env_chk_seq[j] & 0x04)	//水中
			{
				pos.y = info.pos.y;
				if(pos.y < height + 1.0f) pos.y = height + 1.0f;
				if(pos.y > waterheight - 1.0f) pos.y = waterheight - 1.0f;
				if(pos.y >= height + 1.0f && path_finding::IsValidSPPos(_imp->_plane, pos)) break;
			}
		}
		if(j<3) return CreateNPC(pos,param);
	}
	return -1;
}

bool 
object_interface::TestCoolDown(unsigned short id)
{
	return _imp->CheckCoolDown((int)id);
}

void 
object_interface::SetCoolDown(unsigned short id, int ms)
{
	return _imp->SetCoolDown((int)id,ms);
}

void 
object_interface::ResetCoolDown(unsigned short id, int ms)
{
	if(!_imp->CheckCoolDown((int)id))
	{
		_imp->SetCoolDown((int)id,ms>0 ? ms : 1);
	}
}

void
object_interface::SendReduceCDMsg(const XID & target,int skill_id, int msec)
{
	MSG msg;
	msg_reduce_cd_t data;
	data.skill_id = skill_id;
	data.msec = msec;
	BuildMessage(msg,GM_MSG_REDUCE_CD,target,_imp->_parent->ID,_imp->_parent->pos,0,&data,sizeof(data));
	_imp->_plane->PostLazyMessage(msg);
}

bool 
object_interface::TestCommonCoolDown(unsigned short i)
{
	return _imp->CheckCoolDown((int)(i+COOLDOWN_INDEX_COMMONCOOLDOWNSTART));
}

void 
object_interface::SetCommonCoolDown(unsigned short i, int ms)
{
	return _imp->SetCoolDown((int)(i+COOLDOWN_INDEX_COMMONCOOLDOWNSTART),ms);
}

void 
object_interface::EnhanceBreathCapacity(int value)
{
	return _imp->EnhanceBreathCapacity(value);
}

void 
object_interface::ImpairBreathCapacity(int value)
{
	_imp->ImpairBreathCapacity(value);
}

void 
object_interface::ReturnToTown()
{
	_imp->ObjReturnToTown();
}

bool
object_interface::CanReturnToTown()
{
	return true;
}

void 
object_interface::AddPlayerEffect(short effect)
{
	_imp->AddEffectData(effect);
}

void 
object_interface::RemovePlayerEffect(short effect)
{
	_imp->RemoveEffectData(effect);
}

void 
object_interface::AddMultiObjEffect(const XID& target, char type)
{
	_imp->AddMultiObjEffect(target, type);
}

void 
object_interface::RemoveMultiObjEffect(const XID& target, char type)
{
	_imp->RemoveMultiObjEffect(target, type);
}

int 
object_interface::GetImmuneMask()
{
	return _imp->_immune_state | _imp->_immune_state_adj;
}

/*不用了，改为计数方式了
void 
object_interface::SetImmuneMask(int mask)
{
	_imp->_immune_state_adj |= mask;
}
	
void 
object_interface::ClearImmuneMask(int mask)
{
	_imp->_immune_state_adj &= ~mask;
}*/

void 
object_interface::IncImmuneMask(int mask)
{
	_imp->IncImmuneMask(mask);	
}

void 
object_interface::DecImmuneMask(int mask)
{
	_imp->DecImmuneMask(mask);	
}

void 
object_interface::ModifyAP(int ap)
{
	_imp->ModifyAP(ap);
}

void
object_interface::ModifyScaleHP(int hp)
{
    _imp->ModifyScaleHP(hp);
}

bool 
object_interface::SkillMove(const A3DVECTOR & pos, bool notarget)
{
	//检查目标点是否在地下,
	if(_imp->_plane->GetHeightAt(pos.x,pos.z) > pos.y + 0.2f) return false;
	if(world_manager::GetWorldLimit().height_limit)
	{
		if(pos.y >= world_manager::GetHeightLimit() )
		{
			return false;
		}
	}

	trace_manager2 & man = *(_imp->_plane->GetTraceMan());
	if(man.Valid())
	{
		//检查目标点是否嵌入建筑
		bool is_solid;  
		float ratio;    
		bool bRst = man.AABBTrace(pos, A3DVECTOR(0,-1,0), A3DVECTOR(0.3,0.5,0.3), is_solid,ratio,&_imp->_plane->w_collision_flags);
		if(bRst && is_solid) return false;
	}

	A3DVECTOR offset = pos;
	offset -= _imp->_parent->pos;
	if(notarget && offset.y > 5.0f)
	{
		if(offset.y*offset.y > 3*(offset.x*offset.x + offset.z*offset.z))	//限制小于60度
		{
			return false;
		}
	}
	_imp->StepMove(offset);
	_imp->ClearNextSession();
	int seq = _imp->_commander->GetCurMoveSeq() + 200;
	_imp->_runner->change_move_seq(seq & 0xFFFF);
	_imp->_commander->SetNextMoveSeq(seq);
	_imp->PhaseControlInit();
	return true;
}

int 
object_interface::CalcPhysicDamage(int raw_damage, int attacker_level)
{
	float reduce = player_template::GetDamageReduce(_imp->_cur_prop.defense,attacker_level);
	return (int)(raw_damage * (1 - reduce));
}

int 
object_interface::CalcMagicDamage(int dmg_class,int raw_damage, int attacker_level)
{
	float reduce = player_template::GetDamageReduce(_imp->_cur_prop.resistance[dmg_class],attacker_level);
	return (int)(raw_damage * (1 - reduce));
}

float 
object_interface::CalcLevelDamagePunish(int atk_level , int def_level)
{
	float adj = 1.0f;
	int delta = atk_level - def_level;
	player_template::GetAttackLevelPunishment(delta,adj);
	return adj;
}

int 
object_interface::CalcPenetrationEnhanceDamage(int penetration, int raw_damage)
{
	float enhance = player_template::GetPenetrationEnhance(penetration);
	return (int)(raw_damage * (1.f + enhance));
}

int
object_interface::CalcVigourEnhanceDamage(int atk_vigour,int def_vigour,bool pvp, int raw_damage)
{
	float enhance = player_template::GetVigourEnhance(atk_vigour, def_vigour, pvp ? 1 : 4);
	return (int)(raw_damage * enhance);
}

bool 
object_interface::IsInvisible()
{
	return ((gactive_object*)(_imp->_parent))->IsInvisible();	
}

void 
object_interface::SetInvisible(int invisible_degree)
{
	_imp->SetInvisible(invisible_degree);	
}

void 
object_interface::ClearInvisible()
{
	_imp->ClearInvisible();
}

void 
object_interface::SetInvisibleFilter(bool is_invisible,int time, int mana_per_sec,int invisible_degree,int speeddown)
{
	if(!IsPlayerClass()) return;

	if(is_invisible)
	{
		AddFilter(new invisible_filter(_imp,time,mana_per_sec,invisible_degree,speeddown));	
	}
	else
	{
		RemoveFilter(FILTER_INDEX_INVISIBLE);
	}
}

bool 
object_interface::IsGMInvisible()
{
	return _imp->_runner->is_gm_invisible();	
}

void 
object_interface::SetGMInvisible()
{
	_imp->SetGMInvisible();	
}

void 
object_interface::ClearGMInvisible()
{
	_imp->ClearGMInvisible();
}

void 
object_interface::SetGMInvisibleFilter(bool is_invisible,int time, int mask)
{
	if(!IsPlayerClass()) return;

	if(is_invisible)
	{
		AddFilter(new gm_invisible_filter(_imp,time,mask));	
	}
	else
	{
		RemoveFilter(FILTER_INDEX_GM_INVISIBLE);
	}
}

bool 
object_interface::ActivateSharpener(int id, int equip_index)
{
	return _imp->ActivateSharpener(id, equip_index);
}
	
void 
object_interface::TransferSpecFilters(int filter_mask, const XID & target, int count)	
{
	return _imp->TransferSpecFilters(filter_mask, target, count);
}

void 
object_interface::AbsorbSpecFilters(int filter_mask, const XID & target, int count)	
{
	return _imp->AbsorbSpecFilters(filter_mask, target, count);
}

bool 
object_interface::SummonPet2(int pet_egg_id, int skill_level, int life_time)
{
	return _imp->SummonPet2(pet_egg_id, skill_level, life_time);
}

bool 
object_interface::PetSacrifice()
{
	return _imp->PetSacrifice();
}

bool 
object_interface::SummonPlantPet(int pet_egg_id, int skill_level, int life_time, const XID & target, char force_attack)
{
	return _imp->SummonPlantPet(pet_egg_id, skill_level, life_time, target, force_attack);
}

bool 
object_interface::PlantSuicide(float distance, const XID & target, char force_attack)
{
	return _imp->PlantSuicide(distance,target,force_attack);
}

void 
object_interface::InjectPetHPMP(int hp, int mp)
{
	return _imp->InjectPetHPMP(hp,mp);
}

void 
object_interface::DrainPetHPMP(const XID & pet_id, int hp, int mp)
{
	return _imp->DrainPetHPMP(pet_id,hp,mp);
}

void 
object_interface::DrainLeaderHPMP(const XID & attacker, int hp, int mp)
{
	return _imp->DrainLeaderHPMP(attacker,hp,mp);
}

void 
object_interface::LongJump(A3DVECTOR & pos, int tag)
{
	if(IsPlayerClass()) _imp->SendTo<0>(GM_MSG_LONGJUMP, _imp->_parent->ID, tag, &pos, sizeof(pos));
}

void 
object_interface::LongJumpToSpouse()
{
	return _imp->LongJumpToSpouse();
}

void 
object_interface::ShortJump(float distance, bool is_circle)
{
	A3DVECTOR offset(0,0,0);
	if(is_circle)
	{
		float longitude = abase::Rand(0.f, 2*PI );				// (0, 2PI)
		distance = abase::Rand(0.f, distance);
		offset.x = distance * cos(longitude);
		offset.z = distance * sin(longitude);
	}
	else
	{
		offset = _imp->_direction;
		offset.normalize();
		offset *= distance;
	}

	A3DVECTOR newpos = _imp->_parent->pos;
	newpos += offset;
	_imp->Teleport2(newpos, 0, 0);
}	

void 
object_interface::WeaponDisabled(bool disable)
{
	return _imp->WeaponDisabled(disable);
}

void 
object_interface::DetectInvisible(float range)
{
	return _imp->DetectInvisible(range);
}

void 
object_interface::ForceSelectTarget(const XID & target)
{
	return _imp->ForceSelectTarget(target);
}

void 
object_interface::ExchangePosition(const XID & target)
{
	return _imp->ExchangePosition(target);
}

void 
object_interface::SetSkillAttackTransmit(const XID & target)
{
	return _imp->SetSkillAttackTransmit(target);
}

void 
object_interface::ForbidBeSelected(bool b)
{
	if(b) _imp->ClearSubscibeList();
	_imp->ForbidBeSelected(b);
}

void 
object_interface::CallUpTeamMember(const XID& member)
{
	if(_imp->GetPlayerLimit(PLAYER_LIMIT_NOLONGJUMP))//禁止使用巫师召唤
	{
		return;
	}
	return _imp->CallUpTeamMember(member);
}

void 
object_interface::QueryOtherInventory(const XID& target)
{
	return _imp->QueryOtherInventory(target);
}

void 
object_interface::TurretOutOfControl()
{
	_imp->TurretOutOfControl();
}

void 
object_interface::EnterNonpenaltyPVPState(bool b)
{
	_imp->EnterNonpenaltyPVPState(b);
}

void 
object_interface::SetNonpenaltyPVPFilter(bool b, int time)
{
	if(b)
		AddFilter(new nonpenalty_pvp_filter(_imp, time));
	else
		RemoveFilter(FILTER_INDEX_NONPENALTY_PVP);
}

void 
object_interface::GiveMafiaPvPAward(const XID & target, int type)
{
	_imp->FestiveAward(FAT_MAFIA_PVP,type,target);	
}

void
object_interface::RequestPunish(const XID & target, int skillid, int skilllevel)
{
	msg_punish_me_t data = {skillid, skilllevel};
	MSG msg;
	BuildMessage(msg,GM_MSG_PUNISH_ME,target,_imp->_parent->ID,_imp->_parent->pos,0,&data,sizeof(data));
	_imp->_plane->PostLazyMessage(msg);
	
}

int 
object_interface::ChangeVisibleTypeId(int tid)
{
	return _imp->ChangeVisibleTypeId(tid);	
}

bool
object_interface::ModifyFilter(int filterid, int ctrlname, void * ctrlval, size_t ctrllen)
{
	return _imp->_filters.ModifyFilter(filterid, ctrlname, ctrlval, ctrllen);
}

void 
object_interface::SetInfectSkill(int skill,int level)
{
	_imp->SetInfectSkill(skill,level);
}

void
object_interface::ClrInfectSkill(int skill)
{
	if(_imp->GetInfectLevel(skill) != -1)
		_imp->SetInfectSkill(0,0);
}

void 
object_interface::SetSoloChallengeFilterData(int filter_id, int num)
{
	((gplayer_imp *)_imp) ->_solochallenge.SetFilterData(filter_id, num, (gplayer_imp *)_imp);
}

bool 
object_interface::IsPlayerClass()
{
	return _imp->IsPlayerClass();
}

void 
object_interface::BreakCurAction()
{
	_imp->TryStopCurSession();
	_imp->TryBreakAction();
}

bool
object_interface::CreatePet(const A3DVECTOR & pos, char inhabit_mode, const pet_data * pData ,unsigned char pet_stamp,int honor_level, XID & id,char aggro_state, char stay_state)
{
	int cid[3] = {CLS_PET_IMP,CLS_PET_DISPATCHER,CLS_NPC_CONTROLLER};
	if(pData->pet_class == pet_data::PET_CLASS_FOLLOW)
	{
		cid[0] = CLS_PET_IMP_2;
	}
	unsigned char dir = _imp->_parent->dir;
	gnpc * pNPC = npc_spawner::CreatePetBase((gplayer_imp*)_imp,pData,pos,inhabit_mode,cid, 
			dir,pet_stamp,CLS_NPC_AI_POLICY_PET,AGGRO_POLICY_PET);
	if(pNPC)
	{
		gpet_imp * pImp = (gpet_imp*)pNPC->imp;
		pImp->_leader_id = _imp->_parent->ID;
		pImp->SetHonorLevel(honor_level);
		pImp->SetAggroState(aggro_state);
		pImp->SetStayState(stay_state);

		pImp->_money_scale = 0;
		pImp->_drop_rate = 0;
		
		pNPC->master_id = _imp->_parent->ID.id;
		pNPC->object_state |= gactive_object::STATE_NPC_PET;

		if(pData->name_len)
		{
			size_t len = pData->name_len;
			if(len  >sizeof(pData->name))
			{
				ASSERT(false);
				len = sizeof(pData->name);
			}
			memcpy(pNPC->npc_name, pData->name,len);
			pNPC->name_size = len;
			pNPC->object_state |= gactive_object::STATE_NPC_NAME;
		}

		//设置隐身
		gplayer * pPlayer = (gplayer*)_imp->_parent;
		pNPC->invisible_degree = pImp->_invisible_active = pPlayer->invisible_degree;
		pNPC->anti_invisible_degree = pImp->_anti_invisible_active = pPlayer->anti_invisible_degree;
		if(pNPC->invisible_degree)
		{
			pNPC->object_state |= gactive_object::STATE_INVISIBLE;
			__PRINTF("初始化隐身pet,隐身级别%d\n",pNPC->invisible_degree);
		}
	
		_imp->_plane->InsertNPC(pNPC);
		pImp->OnNpcEnterWorld();
		pImp->_runner->enter_world();
		id = pNPC->ID;
		pNPC->Unlock();
		return true;
	}
	return false;
}

bool
object_interface::CreatePet2(const A3DVECTOR & pos, char inhabit_mode, const pet_data * pData ,unsigned char pet_stamp,int honor_level, XID & id,char aggro_state, char stay_state, int skill_level, extend_prop * pProp)
{
	//根据宠物类型选择cid ai_policy_id aggro_policy_id，此函数只能用于生成召唤物和植物
	int cid[3] = {CLS_PET_IMP,CLS_PET_DISPATCHER,CLS_NPC_CONTROLLER};
	int ai_policy_id = CLS_NPC_AI_POLICY_PET; 
	int aggro_policy_id = AGGRO_POLICY_PET;
	if(pData->pet_class == pet_data::PET_CLASS_SUMMON)
	{
	}
	else if(pData->pet_class == pet_data::PET_CLASS_PLANT)
	{
		cid[0] = CLS_PLANT_PET_IMP; 
		ai_policy_id = CLS_NPC_AI_POLICY_PLANT_PET;	
	}
	else
		return false;
		
	unsigned char dir = _imp->_parent->dir;
	gnpc * pNPC = npc_spawner::CreatePetBase2((gplayer_imp*)_imp,pData,pos,inhabit_mode,cid, 
			dir,pet_stamp,ai_policy_id,aggro_policy_id,skill_level);
	if(pNPC)
	{
		gpet_imp * pImp = (gpet_imp*)pNPC->imp;
		pImp->_leader_id = _imp->_parent->ID;
		pImp->SetHonorLevel(honor_level);
		pImp->SetAggroState(aggro_state);
		pImp->SetStayState(stay_state);

		pImp->_money_scale = 0;
		pImp->_drop_rate = 0;
		
		pNPC->master_id = _imp->_parent->ID.id;
		pNPC->object_state |= gactive_object::STATE_NPC_PET;

		if(pData->name_len)
		{
			size_t len = pData->name_len;
			if(len  >sizeof(pData->name))
			{
				ASSERT(false);
				len = sizeof(pData->name);
			}
			memcpy(pNPC->npc_name, pData->name,len);
			pNPC->name_size = len;
			pNPC->object_state |= gactive_object::STATE_NPC_NAME;
		}

		//设置隐身
		gplayer * pPlayer = (gplayer*)_imp->_parent;
		pNPC->invisible_degree = pImp->_invisible_active = pPlayer->invisible_degree;
		pNPC->anti_invisible_degree = pImp->_anti_invisible_active = pPlayer->anti_invisible_degree;
		if(pNPC->invisible_degree)
		{
			pNPC->object_state |= gactive_object::STATE_INVISIBLE;
			__PRINTF("初始化隐身pet,隐身级别%d\n",pNPC->invisible_degree);
		}
	
		_imp->_plane->InsertNPC(pNPC);
		pImp->OnNpcEnterWorld();
		pImp->_runner->enter_world();
		id = pNPC->ID;
		if(pProp) *pProp = pImp->_cur_prop;
		pNPC->Unlock();
		return true;
	}
	return false;
}

bool
object_interface::CreatePet3(const A3DVECTOR & pos, char inhabit_mode, const pet_data * pData ,unsigned char pet_stamp,int honor_level, XID & id,char aggro_state, char stay_state,extend_prop * pProp)
{
	int cid[3] = {CLS_PET_IMP,CLS_PET_DISPATCHER,CLS_NPC_CONTROLLER};
	unsigned char dir = _imp->_parent->dir;
	gnpc * pNPC = npc_spawner::CreatePetBase3((gplayer_imp*)_imp,pData,pos,inhabit_mode,cid, 
			dir,pet_stamp,CLS_NPC_AI_POLICY_PET,AGGRO_POLICY_PET);
	if(pNPC)
	{
		gpet_imp * pImp = (gpet_imp*)pNPC->imp;
		pImp->_leader_id = _imp->_parent->ID;
		pImp->SetHonorLevel(honor_level);
		pImp->SetAggroState(aggro_state);
		pImp->SetStayState(stay_state);

		pImp->_money_scale = 0;
		pImp->_drop_rate = 0;
		
		pNPC->master_id = _imp->_parent->ID.id;
		pNPC->object_state |= gactive_object::STATE_NPC_PET;

		if(pData->name_len)
		{
			size_t len = pData->name_len;
			if(len  >sizeof(pData->name))
			{
				ASSERT(false);
				len = sizeof(pData->name);
			}
			memcpy(pNPC->npc_name, pData->name,len);
			pNPC->name_size = len;
			pNPC->object_state |= gactive_object::STATE_NPC_NAME;
		}

		//设置隐身
		gplayer * pPlayer = (gplayer*)_imp->_parent;
		pNPC->invisible_degree = pImp->_invisible_active = pPlayer->invisible_degree;
		pNPC->anti_invisible_degree = pImp->_anti_invisible_active = pPlayer->anti_invisible_degree;
		if(pNPC->invisible_degree)
		{
			pNPC->object_state |= gactive_object::STATE_INVISIBLE;
			__PRINTF("初始化隐身pet,隐身级别%d\n",pNPC->invisible_degree);
		}
	
		_imp->_plane->InsertNPC(pNPC);
		pImp->OnNpcEnterWorld();
		pImp->_runner->enter_world();
		id = pNPC->ID;
		if(pProp) *pProp = pImp->_cur_prop;
		pNPC->Unlock();
		return true;
	}
	return false;
}
/*
bool
object_interface::CreatePet(const A3DVECTOR & pos, const  object_interface::pet_param & param,XID & id)
{
	npc_template * pTemplate = npc_stubs_manager::Get(param.mob_id);
	if(!pTemplate) return false;
	if(pTemplate->mine_info.is_mine || pTemplate->npc_data.is_npc) return false;
	
	npc_spawner::entry_t ent;
	memset(&ent,0,sizeof(ent));
	ent.npc_tid = param.mob_id;
	ent.ask_for_help = true;
	ent.monster_faction_ask_help = 0;
	ent.accept_ask_for_help = true;
	ent.monster_faction_accept_for_help = _imp->GetMonsterFaction();

	ent.mobs_count = 1;
	ent.msg_mask_or = 0;
	ent.msg_mask_and = 0xFFFFFFFF;

	const int cid[3] = {CLS_PET_IMP,CLS_PET_DISPATCHER,CLS_NPC_CONTROLLER};
	gnpc * pNPC = npc_spawner::CreateMobBase(NULL,_imp->_plane,ent,-1,-1,pos,cid,1,abase::Rand(0,255),CLS_NPC_AI_POLICY_MINOR,AGGRO_POLICY_PET);
	if(pNPC)
	{
		gnpc_imp * pImp = (gnpc_imp*)pNPC->imp;
		pImp->_leader_id = _imp->_parent->ID;

		pImp->_faction = _imp->_faction;
		pImp->_enemy_faction = _imp->GetEnemyFaction();
		pNPC->monster_faction |= _imp->GetFactionAskHelp();

		pImp->_basic.exp = param.exp;
		if(pImp->_basic.exp < 0) pImp->_basic.exp = 0;

		pImp->_basic.skill_point = param.sp;
		if(pImp->_basic.skill_point < 0) pImp->_basic.skill_point = 0;

		pImp->_money_scale = 0;
		pImp->_drop_rate = 0;
		
		pImp->SetDieWithLeader(true);
		pNPC->vis_tid = param.vis_id;
		pNPC->body_size = param.body_size;

		pNPC->master_id = _imp->_parent->ID.id;
		pNPC->object_state |= gactive_object::STATE_NPC_PET;
		
		if(param.use_pet_name)
		{
			ASSERT(param.pet_name_size <= sizeof(param.pet_name)); 
			memcpy(pNPC->npc_name, param.pet_name,param.pet_name_size);
			pNPC->name_size = param.pet_name_size;
			pNPC->object_state |= gactive_object::STATE_NPC_NAME;
		}

		_imp->_plane->InsertNPC(pNPC);
		pImp->_runner->enter_world();
		id = pNPC->ID;
		pNPC->Unlock();
		return true;
	}
	return false;
}
*/

bool 
object_interface::IsEquipWing()
{
	return _imp->IsEquipWing();
}

int object_interface::GetLinkIndex()
{
	return _imp->GetLinkIndex();
}

int object_interface::GetLinkSID()
{
	return _imp->GetLinkSID();
}

int object_interface::GetSystime()
{
	return g_timer.get_systime();
}

size_t object_interface::GetInventorySize()
{
	return _imp->OI_GetInventorySize();
}

size_t object_interface::GetEmptySlotSize()
{
	return _imp->OI_GetEmptySlotSize();
}

int object_interface::GetInventoryDetail(GDB::itemdata * list, size_t size)
{
	return _imp->OI_GetInventoryDetail(list,size);
}

int object_interface::GetTrashBoxDetail(GDB::itemdata * list, size_t size)
{
	return _imp->OI_GetTrashBoxDetail(gplayer_imp::IL_TRASH_BOX, list,size);
}

int object_interface::GetTrashBox2Detail(GDB::itemdata * list, size_t size)
{
	return _imp->OI_GetTrashBoxDetail(gplayer_imp::IL_TRASH_BOX2, list,size);
}

int object_interface::GetTrashBox3Detail(GDB::itemdata * list, size_t size)
{
	return _imp->OI_GetTrashBoxDetail(gplayer_imp::IL_TRASH_BOX3, list,size);
}

int object_interface::GetTrashBox4Detail(GDB::itemdata * list, size_t size)
{
	return _imp->OI_GetTrashBoxDetail(gplayer_imp::IL_TRASH_BOX4, list,size);
}

int object_interface::GetEquipmentDetail(GDB::itemdata * list, size_t size)
{
	return _imp->OI_GetEquipmentDetail(list,size);
}

size_t object_interface::GetEquipmentSize()
{
	return _imp->OI_GetEquipmentSize();
}

size_t object_interface::GetTrashBoxCapacity()
{
	return _imp->OI_GetTrashBoxCapacity(gplayer_imp::IL_TRASH_BOX);
}

size_t object_interface::GetTrashBox2Capacity()
{
	return _imp->OI_GetTrashBoxCapacity(gplayer_imp::IL_TRASH_BOX2);
}

size_t object_interface::GetTrashBox3Capacity()
{
	return _imp->OI_GetTrashBoxCapacity(gplayer_imp::IL_TRASH_BOX3);
}

size_t object_interface::GetTrashBox4Capacity()
{
	return _imp->OI_GetTrashBoxCapacity(gplayer_imp::IL_TRASH_BOX4);
}

size_t object_interface::GetTrashBoxMoney()
{
	return _imp->OI_GetTrashBoxMoney();
}

bool object_interface::IsTrashBoxModified()
{
	return _imp->OI_IsTrashBoxModified();
}

bool object_interface::IsEquipmentModified()
{
	return _imp->OI_IsEquipmentModified();
}

int object_interface::TradeLockPlayer(int get_mask, int put_mask)
{
	return _imp->TradeLockPlayer(get_mask, put_mask);
}

int object_interface::TradeUnLockPlayer()
{
	return _imp->TradeUnLockPlayer();
}


void 
object_interface::SendClientAttackData()
{
//	_imp->_runner->get_extprop_attack();
	_imp->_runner->self_get_property(_imp->_basic.status_point,_imp->_cur_prop,
									_imp->_attack_degree, _imp->_defend_degree, 
									_imp->_crit_rate+_imp->_base_crit_rate, _imp->_crit_damage_bonus,
									((gactive_object*)_imp->_parent)->invisible_degree, ((gactive_object*)_imp->_parent)->anti_invisible_degree,
									_imp->_penetration, _imp->_resilience, _imp->GetVigour(), _imp->_anti_defense_degree, _imp->_anti_resistance_degree, ((gplayer_imp*)_imp)->_rank_kill, ((gplayer_imp*)_imp)->_rank_dead);
}

void 
object_interface::SendClientDefenseData()
{
//	_imp->_runner->get_extprop_defense();
	_imp->_runner->self_get_property(_imp->_basic.status_point,_imp->_cur_prop,
									_imp->_attack_degree, _imp->_defend_degree, 
									_imp->_crit_rate+_imp->_base_crit_rate, _imp->_crit_damage_bonus,
									((gactive_object*)_imp->_parent)->invisible_degree, ((gactive_object*)_imp->_parent)->anti_invisible_degree,
									_imp->_penetration, _imp->_resilience, _imp->GetVigour(), _imp->_anti_defense_degree, _imp->_anti_resistance_degree, ((gplayer_imp*)_imp)->_rank_kill, ((gplayer_imp*)_imp)->_rank_dead);
}

void 
object_interface::DuelStart(const XID & target)
{
	_imp->OnDuelStart(target);
}

void 
object_interface::DuelStop()
{
	_imp->OnDuelStop();
}

void 
object_interface::SendClientDuelStart(const XID & target)
{
	_imp->_runner->duel_start(target);
	__PRINTF("决斗开始\n");
}

void 
object_interface::SendClientDuelStop(const XID & target)
{
	_imp->_runner->duel_stop();
	__PRINTF("决斗结束\n");
}

void
object_interface::SendClientScreenEffect(int effectid, int state)
{
	_imp->_runner->player_screen_effect_notify(gplayer_imp::CLIENT_SCREEN_EFFECT,effectid, state);
}

void
object_interface::SendClientGfxEffect(int effectid, int state)
{
	_imp->_runner->player_screen_effect_notify(gplayer_imp::CLIENT_GFX_EFFECT,effectid, state);
}

void
object_interface::SendClientComboSkillPerpare(int skillid,int timestamp,int arg1, int arg2, int arg3)
{
	_imp->_runner->player_combo_skill_prepare(skillid,timestamp,arg1,arg2,arg3);
}

void
object_interface::SendClientPrayDistanceChange(float pd)
{
	_imp->_runner->player_pray_distance_change(pd);
}

void 
object_interface::ActiveMountState(int mount_id, unsigned short mount_color)
{
	_imp->ActiveMountState(mount_id, mount_color);
}

void 
object_interface::DeactiveMountState()
{
	_imp->DeactiveMountState();
}

bool 
object_interface::CheckGMPrivilege()
{
	return _imp->CheckGMPrivilege();
}

int 
object_interface::GetDBTimeStamp()
{
	return _imp->OI_GetDBTimeStamp();
}

int 
object_interface::InceaseDBTimeStamp()
{
	return _imp->OI_InceaseDBTimeStamp();
}
	
size_t 
object_interface::GetSkillDataSize()
{
	return _imp->_skill.StoreDatabaseSize();
}

bool 
object_interface::GetSkillData(void * buf, size_t size)
{
	raw_wrapper ar;
	_imp->_skill.StoreDatabase(ar);
	if(size != ar.size()) return false;
	memcpy(buf, ar.data(), ar.size());
	return true;
}

size_t object_interface::GetPetsCount()
{
	return _imp->OI_GetPetsCount();
}

bool 
object_interface::GetPetsData(GDB::petcorral pets)
{
	if(pets.count != _imp->OI_GetPetsCount())
		return false;
	if(pets.count)
	{
		size_t pet_count = 0;
		for(size_t i=0; i<pet_manager::MAX_PET_CAPACITY; i++)
		{
			pet_data * pData = _imp->OI_GetPetData(i);
			if(!pData) continue;
			pets.list[pet_count].index = i;
			pets.list[pet_count].data.data = pData;
			pets.list[pet_count].data.size = sizeof(pet_data);
			pet_count ++;
		}
	}
	pets.capacity = _imp->OI_GetPetSlotCapacity();
	return true;
}

void 
object_interface::EnhanceCrit(int val)
{
	_imp->_crit_rate += val;
	property_policy::UpdateCrit(_imp);
}


void 
object_interface::ImpairCrit(int val)
{
	_imp->_crit_rate -= val;
	property_policy::UpdateCrit(_imp);
}

void 
object_interface::EnhanceCritDamage(int percent)
{
	_imp->_crit_damage_bonus += percent;
}


void 
object_interface::ImpairCritDamage(int percent)
{
	_imp->_crit_damage_bonus -= percent;
}

void 
object_interface::EnhanceCritDamageReduce(int percent)
{
	_imp->_crit_damage_reduce += percent;
}

void 
object_interface::ImpairCritDamageReduce(int percent)
{
	_imp->_crit_damage_reduce -= percent;
}

void 
object_interface::EnhanceCritResistance(int val)
{
	_imp->_crit_resistance += val;
}

void 
object_interface::ImpairCritResistance(int val)
{
	_imp->_crit_resistance -= val;
}

void 
object_interface::IncAntiInvisiblePassive(int val)
{
	_imp->IncAntiInvisiblePassive(val);
}

void 
object_interface::DecAntiInvisiblePassive(int val)
{
	_imp->DecAntiInvisiblePassive(val);
}

void 
object_interface::IncAntiInvisibleActive(int val)
{
	_imp->IncAntiInvisibleActive(val);
}

void 
object_interface::DecAntiInvisibleActive(int val)
{
	_imp->DecAntiInvisibleActive(val);
}

void 
object_interface::IncInvisiblePassive(int val)
{
	_imp->IncInvisiblePassive(val);
}

void 
object_interface::DecInvisiblePassive(int val)
{
	_imp->DecInvisiblePassive(val);
}

void 
object_interface::EnhanceDamageDodge(int val)
{
	_imp->_damage_dodge_rate += val;	
}

void 
object_interface::ImpairDamageDodge(int val)
{
	_imp->_damage_dodge_rate -= val;	
}
	
void 
object_interface::EnhanceDebuffDodge(int val)
{
	_imp->_debuff_dodge_rate += val;
}
	
void 
object_interface::ImpairDebuffDodge(int val)
{
	_imp->_debuff_dodge_rate -= val;
}

void 
object_interface::EnhanceHpSteal(int val)
{
	_imp->_hp_steal_rate += val;	
}

void 
object_interface::ImpairHpSteal(int val)
{
	_imp->_hp_steal_rate -= val;	
}

void 
object_interface::IncHealCoolTime(int ms)
{
	_imp->_heal_cool_time_adj += ms;
}

void 
object_interface::DecHealCoolTime(int ms)
{
	_imp->_heal_cool_time_adj -= ms;
}

void 
object_interface::IncAggroOnDamage(const XID & attacker, int val)
{
	_imp->IncAggroOnDamage(attacker,val);
}

void 
object_interface::DecAggroOnDamage(const XID & attacker, int val)
{
	_imp->DecAggroOnDamage(attacker,val);
}

void 
object_interface::EnhanceSkillDamage(int value)
{
	_imp->_skill_enhance += value;
}

void 
object_interface::ImpairSkillDamage(int value)
{
	_imp->_skill_enhance -= value;
}

void 
object_interface::EnhanceSkillDamage2(int value)
{
	_imp->_skill_enhance2 += value;
}

void 
object_interface::ImpairSkillDamage2(int value)
{
	_imp->_skill_enhance2 -= value;
}

void 
object_interface::IncApPerHit(int value)
{
	_imp->ModifyPerHitAP(value);
}

void 
object_interface::DecApPerHit(int value)
{
	_imp->ModifyPerHitAP(-value);
}

void 
object_interface::ReduceResurrectExpLost(int value)
{
	_imp->ReduceResurrectExpLost(value);
}

void 
object_interface::IncreaseResurrectExpLost(int value)
{
	_imp->IncreaseResurrectExpLost(value);
}

void
object_interface::IncPenetration(int val)
{
	_imp->_penetration += val;
}

void 
object_interface::DecPenetration(int val)
{
	_imp->_penetration -= val;
}

void
object_interface::IncVigour(int val)
{
	_imp->EnhanceVigour(val);
}

void 
object_interface::DecVigour(int val)
{
	_imp->ImpairVigour(val);
}

void 
object_interface::EnhanceNearNormalDmgReduce(float scale)
{
	_imp->_near_normal_dmg_reduce += scale;
}

void 
object_interface::ImpairNearNormalDmgReduce(float scale)
{
	_imp->_near_normal_dmg_reduce -= scale;
}

void 
object_interface::EnhanceNearSkillDmgReduce(float scale)
{
	_imp->_near_skill_dmg_reduce += scale;
}

void 
object_interface::ImpairNearSkillDmgReduce(float scale)
{
	_imp->_near_skill_dmg_reduce -= scale;
}

void 
object_interface::EnhanceFarNormalDmgReduce(float scale)
{
	_imp->_far_normal_dmg_reduce += scale;
}

void 
object_interface::ImpairFarNormalDmgReduce(float scale)
{
	_imp->_far_normal_dmg_reduce -= scale;
}

void 
object_interface::EnhanceFarSkillDmgReduce(float scale)
{
	_imp->_far_skill_dmg_reduce += scale;
}

void 
object_interface::ImpairFarSkillDmgReduce(float scale)
{
	_imp->_far_skill_dmg_reduce -= scale;
}

void
object_interface::IncResilience(int val)
{
	_imp->_resilience += val;
}

void
object_interface::DecResilience(int val)
{
	_imp->_resilience -= val;
}

void object_interface::ImpairPlusDamage(int dmg)
{
	_imp->_plus_enhanced_param.damage -= dmg;
}
void object_interface::EnhancePlusDamage(int dmg)
{
	_imp->_plus_enhanced_param.damage += dmg;
}
void object_interface::ImpairPlusMagicDamage(int dmg)
{
	_imp->_plus_enhanced_param.magic_dmg -= dmg;
}
void object_interface::EnhancePlusMagicDamage(int dmg)
{
	_imp->_plus_enhanced_param.magic_dmg += dmg;
}
void object_interface::ImpairPlusDefense(int defence)
{
	_imp->_plus_enhanced_param.defence -= defence;
}
void object_interface::EnhancePlusDefense(int defence)
{
	_imp->_plus_enhanced_param.defence += defence;
}
void object_interface::ImpairPlusResistance(size_t cls, int res)
{
	_imp->_plus_enhanced_param.resistance[cls] -= res;
}
void object_interface::EnhancePlusResistance(size_t cls, int res)
{
	_imp->_plus_enhanced_param.resistance[cls] += res;
}
void object_interface::ImpairPlusMaxHP(int hp, bool update)
{
	_imp->_plus_enhanced_param.max_hp -= hp;
	if(update)
	{
		property_policy::UpdateLife(_imp);
		_imp->SetRefreshState();
	}
}
void object_interface::EnhancePlusMaxHP(int hp)
{
	_imp->_plus_enhanced_param.max_hp += hp;
	property_policy::UpdateLife(_imp);
	_imp->SetRefreshState();
}

bool 
object_interface::CheckWaypoint(int point_index, int & point_domain)
{
	return _imp->CheckWaypoint(point_index, point_domain);
}

void object_interface::ReturnWaypoint(int point)
{
	_imp->ReturnWaypoint(point);
}

int object_interface::GetCityOwner(int city_id)
{
	return GMSV::GetCityOwner(city_id);
}

void object_interface::DropSpecItem(bool isProtected, const XID & owner)
{
	return _imp->DropSpecItem(isProtected, owner);
}

bool object_interface::IsInventoryFull()
{
	return _imp->OI_IsInventoryFull();
}

bool object_interface::IsPet()
{
	return _imp->OI_IsPet();
}

int object_interface::GetPetEggID()
{
	return _imp->OI_GetPetEggID();
}

XID object_interface::GetPetID()
{
	return _imp->OI_GetPetID();
}

void object_interface::ResurrectPet()
{
	return _imp->OI_ResurrectPet();
}

void object_interface::TransferPetEgg(const XID & target, int pet_egg_id)
{
	_imp->OI_TransferPetEgg(target, pet_egg_id);
}

void object_interface::Disappear()
{
	_imp->OI_Disappear();
}

void object_interface::Die()
{
	_imp->OI_Die();
}

bool
object_interface::GetMallInfo(int & cash,int & cash_used, int &cash_delta,  int &order_id)
{
	return _imp->OI_GetMallInfo(cash,cash_used, cash_delta, order_id);
}

bool 
object_interface::IsCashModified()
{
	return _imp->OI_IsCashModified();
}

size_t 
object_interface::GetMallOrdersCount()
{
	return _imp->OI_GetMallOrdersCount();
}

int 
object_interface::GetMallOrders(GDB::shoplog * list, size_t size)
{
	return _imp->OI_GetMallOrders(list, size);
}

bool 
object_interface::TestSafeLock()
{
	return _imp->OI_TestSafeLock();
}


//lgc 小精灵相关
bool object_interface::GetElfProp(short& level, short& str, short& agi, short& vit, short& eng)
{
	return _imp->OI_GetElfProp(level, str, agi, vit, eng);	
	
}
bool object_interface::EnhanceElfProp(short str, short agi, short vit, short eng)
{
	if(str < 0 || agi < 0 || vit < 0 || eng < 0)
		return false;
	_imp->_elf_en.str_point += str;
	_imp->_elf_en.agi_point += agi;
	_imp->_elf_en.vit_point += vit;
	_imp->_elf_en.eng_point += eng;
	if(_imp->IsElfEquipped())
	{
		_imp->UpdateElfProp();
		_imp->UpdateElfVigor();
	}
	return true;	
	
}
bool object_interface::ImpairElfProp(short str, short agi, short vit, short eng)
{
	if(str < 0 || agi < 0 || vit < 0 || eng < 0)
		return false;
	_imp->_elf_en.str_point -= str;
	_imp->_elf_en.agi_point -= agi;
	_imp->_elf_en.vit_point -= vit;
	_imp->_elf_en.eng_point -= eng;
	if(_imp->IsElfEquipped())
	{
		_imp->UpdateElfProp();
		_imp->UpdateElfVigor();
	}
	return true;	
	
}
int object_interface::GetElfVigor()
{
	return _imp->OI_GetElfVigor();	
}
int object_interface::GetElfStamina()
{
	return _imp->OI_GetElfStamina();	
}
bool object_interface::DrainElfVigor(int dec)
{
	return _imp->OI_DrainElfVigor(dec);	
}
bool object_interface::DrainElfStamina(int dec)
{
	return _imp->OI_DrainElfStamina(dec);	
}
void object_interface:: IncPetHp(int inc)
{
	return _imp->IncPetHp(inc);
}
void object_interface:: IncPetMp(int inc)
{
	return _imp->IncPetMp(inc);
}
void object_interface:: IncPetDamage(int inc)
{
	return _imp->IncPetDamage(inc);
}
void object_interface:: IncPetMagicDamage(int inc)
{
	return _imp->IncPetMagicDamage(inc);
}
void object_interface:: IncPetDefense(int inc)
{
	return _imp->IncPetDefense(inc);
}
void object_interface:: IncPetMagicDefense(int inc)
{
	return _imp->IncPetMagicDefense(inc);
}
void object_interface:: IncPetAttackDegree(int inc)
{
	return _imp->IncPetAttackDegree(inc);
}
void object_interface:: IncPetDefendDegree(int inc)
{
	return _imp->IncPetDefendDegree(inc);
}
void object_interface::Say(const char * msg)
{
	if(!IsPlayerClass()) return;
	((gplayer_imp *)_imp)->Say(msg);	
}

int
object_interface::GetSpherePlayerListSize(const A3DVECTOR& pos,float radius)
{
   	return _imp->_plane->GetSpherePlayerListSize(pos,radius);
}
