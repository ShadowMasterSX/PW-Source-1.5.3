#include "string.h"
#include "world.h"
#include "rune_filter.h"
#include "clstab.h"
#include "actobject.h"

DEFINE_SUBSTANCE(defense_rune_filter,filter, CLS_FILTER_DEFENSE_RUNE)
DEFINE_SUBSTANCE(resistance_rune_filter,filter, CLS_FILTER_RESISTANCE_RUNE)

void 
defense_rune_filter::OnAttach()
{
	_parent.GetImpl()->_runner->defense_rune_enabled(RUNE_TYPE_DEFENSE,1);
}

void 
defense_rune_filter::OnRelease()
{
	_parent.GetImpl()->_runner->defense_rune_enabled(RUNE_TYPE_DEFENSE,0);
}

void 
defense_rune_filter::AdjustDamage(damage_entry & ent, const XID &, const attack_msg&, float)
{
	if(ent.physic_damage)
	{
		ent.physic_damage = ent.physic_damage * _damage_adjust;
		if( --_times <= 0) _is_deleted = true;
		//设置必要的标志 好发给客户端一些消息
		_parent.SetATDefenseState(gactive_imp::AT_STATE_DEFENSE_RUNE1);
	}
}

void 
resistance_rune_filter::OnAttach()
{
	_parent.GetImpl()->_runner->defense_rune_enabled(RUNE_TYPE_RESISTANCE,1);
}

void 
resistance_rune_filter::OnRelease()
{
	_parent.GetImpl()->_runner->defense_rune_enabled(RUNE_TYPE_RESISTANCE,0);
}

void 
resistance_rune_filter::AdjustDamage(damage_entry& ent, const XID &, const attack_msg&,float)
{
	if(ent.magic_damage[0]+ent.magic_damage[1]+ent.magic_damage[2]+ent.magic_damage[3]+ent.magic_damage[4])
	{
		ent.magic_damage[0] = ent.magic_damage[0] * _damage_adjust;
		ent.magic_damage[1] = ent.magic_damage[1] * _damage_adjust;
		ent.magic_damage[2] = ent.magic_damage[2] * _damage_adjust;
		ent.magic_damage[3] = ent.magic_damage[3] * _damage_adjust;
		ent.magic_damage[4] = ent.magic_damage[4] * _damage_adjust;
		if( --_times <= 0) _is_deleted = true;
		//设置必要的标志 好发给客户端一些消息
		_parent.SetATDefenseState(gactive_imp::AT_STATE_DEFENSE_RUNE2);
	}
}

