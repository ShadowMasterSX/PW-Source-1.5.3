#include "string.h"
#include "world.h"
#include "skill_filter.h"
#include "invincible_filter.h"
#include "clstab.h"

DEFINE_SUBSTANCE(skill_interrupt_filter,filter,CLS_FILTER_SKILL_INTERRUPT)
DEFINE_SUBSTANCE(gather_interrupt_filter,filter,CLS_FILTER_GATHER_INTERRUPT)
DEFINE_SUBSTANCE(moving_skill_interrupt_filter,filter,CLS_FILTER_MOVING_SKILL_INTERRUPT)

void 
skill_interrupt_filter::AdjustDamage(damage_entry&, const XID &, const attack_msg&,float)
{
	if(_parent.SessionOnAttacked(_session_id))
	{
		_is_deleted = true;
	}
}

bool 
skill_interrupt_filter::Save(archive & ar)
{
	filter::Save(ar);
	ar <<  _session_id;
	return true;
}

bool 
skill_interrupt_filter::Load(archive & ar)
{
	filter::Load(ar);
	ar >> _session_id;
	return true;
}

void 
gather_interrupt_filter::TranslateRecvAttack(const XID & attacker,attack_msg & msg)
{
	if(msg.target_faction & _parent.GetFaction())
	{
		if(_parent.SessionOnAttacked(_session_id))
		{
			_is_deleted = true;
		}
	}
}

bool 
gather_interrupt_filter::Save(archive & ar)
{
	filter::Save(ar);
	ar <<  _session_id;
	return true;
}

bool 
gather_interrupt_filter::Load(archive & ar)
{
	filter::Load(ar);
	ar >> _session_id;
	return true;
}

void 
moving_skill_interrupt_filter::AdjustDamage(damage_entry&, const XID &, const attack_msg&,float)
{
	if(_parent.ActionOnAttacked(_action_id))
	{
		_is_deleted = true;
	}
}

bool 
moving_skill_interrupt_filter::Save(archive & ar)
{
	filter::Save(ar);
	ar << _action_id;
	return true;
}

bool 
moving_skill_interrupt_filter::Load(archive & ar)
{
	filter::Load(ar);
	ar >> _action_id;
	return true;
}

