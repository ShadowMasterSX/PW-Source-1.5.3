#include "types.h"
#include "obj_interface.h"
#include "amemobj.h"
//#include "basicinfo.h"
//#include "myobject.h"


//DEFINE_SUBSTANCE_ABSTRACT(filter, substance, 4095);

namespace abase
{
	void * ASmallObject::operator new(size_t size)
	{
		return (void*)malloc(size);
	}

	void ASmallObject::operator delete(void * p, size_t size)
	{
		free(p);
	}
}
/*
#ifdef __cplusplus
extern "C"
{
#endif
int * ASSERT_FAIL(const char * msg, const char *filename , int line)
{
	fprintf(stderr, "%s %s %d\n", msg, filename, line);
	return NULL;
}
#ifdef __cplusplus
}
#endif
*/
void object_interface::BeHurt(const XID & who,int lid, const attacker_info_t & info, int damage,char attacker_mode )
{ }
void object_interface::BeHurt(const XID & who,int lid, const attacker_info_t & info, int damage,bool invader, char attacker_mode)
{}

void object_interface::Heal(const XID & healer,size_t life)
{
}

void object_interface::Heal(size_t life)
{
}
void object_interface::InjectMana(int mana)
{
}
bool object_interface::DrainMana(int mana)
{
	return true;
}
void object_interface::DecHP(int hp)
{
}
void object_interface::DecSkillPoint(int sp)
{
}
bool object_interface::SessionOnAttacked(int session_id)
{
	return true;
}

void object_interface::SetATDefenseState(char state)
{
}

int object_interface::SpendFlyTime(int tick)
{
	return 0;
}
void object_interface::TakeOff()
{
}
void object_interface::Landing()
{
}
void object_interface::SendClientMsgFlySwordTime(int time)
{

}
void object_interface::AddFilter(filter*)
{
}

int object_interface::GenerateDamage()
{
	return 0;
}
int object_interface::GenerateMagicDamage()
{
	return 0;
}
bool object_interface::UseProjectile(size_t count)
{
	return true;
}
bool object_interface::CanAttack(const XID & target)
{
	return true;
}
void object_interface::Attack(const XID & target, attack_msg & attack,int use_projectile) { }
void object_interface::RegionAttack1(const A3DVECTOR& pos, float radius,attack_msg & attack,int use_projectile) { }
void object_interface::RegionAttack2(const A3DVECTOR& pos, float radius,attack_msg & attack,int use_projectile) { }
void object_interface::RegionAttack3(const A3DVECTOR& pos, float cos_half_angle,attack_msg & attack,int use_projectile)
{}
void object_interface::MultiAttack(const XID * target,size_t size, attack_msg & attack,int use_projectile) {}
void object_interface::MultiAttack(const XID * target,size_t size, attack_msg & attack,MsgAdjust & adj,int use_projectile)
{}
void object_interface::Enchant(const XID & target, enchant_msg & msg)
{
}
void object_interface::EnchantZombie(const XID & target, enchant_msg & msg)
{

}
void object_interface::RegionEnchant1(const A3DVECTOR& pos, float radius,enchant_msg & msg)
{

}
void object_interface::RegionEnchant2(const A3DVECTOR& pos, float radius,enchant_msg & msg )
{
}
void object_interface::RegionEnchant3(const A3DVECTOR& pos, float cos_half_angle,enchant_msg & msg )
{

}
void object_interface::MultiEnchant(const XID * target,size_t size, enchant_msg & msg)
{
}
void object_interface::TeamEnchant(enchant_msg & msg,bool exclude_self)
{

}
void object_interface::EnterCombatState()
{
}
void object_interface::KnockBack(const XID & attacker, const A3DVECTOR &source,float distance)
{
}
bool object_interface::Resurrect(float exp_reduce)
{
	return true;
}
bool object_interface::IsDead()
{
	return true;
}
bool object_interface::IsMember(const XID & member)
{
	return true;
}
bool object_interface::IsInTeam()
{
	return true;
}
bool object_interface::IsTeamLeader()
{
	return true;
}
int  object_interface::GetClass()
{
	return 0;
}
int  object_interface::GetFaction()
{
	return 0;
}
int  object_interface::GetEnemyFaction()
{
	return 0;
}
const A3DVECTOR & object_interface::GetPos()
{
	static A3DVECTOR pos;
	return pos;
}
float object_interface::GetBodySize()
{
	return 0;
}
int object_interface::QueryObject(const XID & who, A3DVECTOR & pos, float & body_size)
{
	return 1;
}
void object_interface::SendClientMsgSkillCasting(const XID &target, int skill, unsigned short time,unsigned char level)
{
}
void object_interface::SendClientMsgSkillInterrupted(char reason)
{
}
void object_interface::SendClientMsgSkillPerform()
{
}
void object_interface::SendClientNotifyRoot(unsigned char)
{
}	
void object_interface::SendClientDispelRoot(unsigned char)
{
}
void object_interface::IncVisibleState(unsigned short state)
{
}
void object_interface::DecVisibleState(unsigned short state)
{
}
void object_interface::ClearVisibleState(unsigned short state)
{
}
void object_interface::InsertTeamVisibleState(unsigned short state)
{
}
void object_interface::RemoveTeamVisibleState(unsigned short state)
{
}
void object_interface::ChangeShape(int shape)
{
}
int object_interface::GetShape()
{
	return 0;
}
void object_interface::SetIdleMode(bool sleep,bool stun)
{
}
void object_interface::SetSealMode(bool silent,bool root)
{
}
void object_interface::GetIdleMode(bool & sleep, bool & stun)
{
}
void object_interface::GetSealMode(bool & silent, bool & root)
{
}
bool object_interface::IsAggressive()
{
	return true;
}
void object_interface::SetAggressive(bool isActive )
{
}
void object_interface::AddAggro(const XID & attacker , int rage)
{
}
void object_interface::AddAggroToEnemy(const XID & helper, int rage)
{
}
void object_interface::EnhanceHPGen(int hpgen)
{
}
void object_interface::ImpairHPGen(int hpgen)
{
}
void object_interface::EnhanceMPGen(int mpgen)
{
}
void object_interface::ImpairMPGen(int mpgen)
{
}
void object_interface::EnhanceMaxHP(int hp)
{
}
void object_interface::ImpairMaxHP(int hp)
{
}
void object_interface::EnhanceMaxMP(int mp)
{
}
void object_interface::ImpairMaxMP(int mp)
{
}
void object_interface::EnhanceDefense(int def)
{
}
void object_interface::ImpairDefense(int def)
{
}
void object_interface::EnhanceScaleDefense(int def)
{
}
void object_interface::ImpairScaleDefense(int def)
{
}
void object_interface::EnhanceArmor(int ac)
{
}
void object_interface::ImpairArmor(int ac)
{
}
void object_interface::EnhanceScaleArmor(int ac)
{
}
void object_interface::ImpairScaleArmor(int ac)
{
}
void object_interface::EnhanceResistance(size_t cls, int res)
{
}
void object_interface::ImpairResistance(size_t cls, int res)
{
}
void object_interface::EnhanceScaleDamage(int dmg)
{
}
void object_interface::ImpairScaleDamage(int dmg)
{
}

void object_interface::EnhanceScaleMagicDamage(int dmg)
{
}
void object_interface::ImpairScaleMagicDamage(int dmg)
{
}

void object_interface::EnhanceSpeed(int speedup)
{
}
void object_interface::ImpairSpeed(int speedup)
{
}


void object_interface::UpdateDefenseData()
{
}
void object_interface::UpdateAttackData()
{
}
void object_interface::UpdateMagicData()
{
}	
void object_interface::UpdateSpeedData()
{
}
int object_interface::TakeOutItem(int item_id)
{
	return 0;
}	
const basic_prop & 		object_interface::GetBasicProp()
{
	static basic_prop p;
	p.level=150;
	p.skill_point=1000000;
	return p;
}
const extend_prop & 		object_interface::GetExtendProp()
{
	static extend_prop p;
	return p;
}
const enhanced_param & 		object_interface::GetEnhancedParam()
{
	static enhanced_param p;
	return p;
}
const scale_enhanced_param & 	object_interface::GetScaleEnhanecdParam()
{
	static scale_enhanced_param p;
	return p;
}
const item_prop &		object_interface::GetCurWeapon()
{
	static item_prop p;
	return p;
}
/*
GNET::SkillWrapper &    object_interface::GetSkillWrapper()
{
	static GNET::SkillWrapper wrapper;
	return wrapper;
}
*/
void object_interface::SendClientCurSpeed()
{
}
void object_interface::SendClientEnchantResult(const XID & caster, int skill, char level, bool invader,char at_state)
{}

void object_interface::EnhanceScaleMaxMP(int mp)
{
}
void object_interface::ImpairScaleMaxMP(int mp)
{
}
void object_interface::EnhanceScaleAttackSpeed(int speedup)
{
}
void object_interface::ImpairScaleAttackSpeed(int speedup)
{
}
void object_interface::EnhanceScaleMaxHP(int hp)
{
}
void object_interface::ImpairScaleMaxHP(int hp)
{
}
void object_interface::SendHealMsg(const XID & target, int hp)
{
}

void object_interface::EnhanceScaleAttack(int attack)
{
}
void object_interface::ImpairScaleAttack(int attack)
{
}
void object_interface::SetRetortState()
{
}
const XID & object_interface::GetSelfID()
{
	static XID xid(1,1);
	return xid;
}
unsigned int object_interface::GetMoney()
{
	return 10000000;
}
size_t object_interface::GetInventorySize()
{
	return 0;
}
int object_interface::GetLinkIndex()
{
	return 0;
}
int object_interface::GetLinkSID()
{
	return 0;
}
int TradeLockPlayer(int get_mask,int put_mask)
{ return 0; }

