#include "string.h"
#include "world.h"
#include "invincible_filter.h"
#include "clstab.h"
#include "actobject.h"

DEFINE_SUBSTANCE(invincible_filter,filter,CLS_FILTER_INVINCIBLE)
DEFINE_SUBSTANCE(invincible_filter_2,invincible_filter,CLS_FILTER_INVINCIBLE_2)
DEFINE_SUBSTANCE(invincible_filter_to_spec_id,filter,CLS_FILTER_INVINCIBLE_SPEC_ID)
DEFINE_SUBSTANCE(invincible_banish_filter,filter,CLS_FILTER_BANISH_INVINCIBLE)

void 
invincible_filter::OnAttach()
{
	gobject * pObj = _parent.GetImpl()->_parent; 
	if( (_isAggressive = pObj->msg_mask & gobject::MSG_MASK_PLAYER_MOVE))
	{
		pObj->msg_mask &= ~gobject::MSG_MASK_PLAYER_MOVE;
	}
	if(!_parent.IsPlayerClass())
	{
		_parent.ClearSpecFilter(filter::FILTER_MASK_DEBUFF);
	}
	_parent.IncVisibleState(49);
}

void 
invincible_filter::OnRelease()
{
	if(_isAggressive)
	{
		gobject * pObj = _parent.GetImpl()->_parent; 
		pObj->msg_mask |= gobject::MSG_MASK_PLAYER_MOVE;
	}
	_parent.DecVisibleState(49);
}

void 
invincible_filter::TranslateRecvAttack(const XID & attacker,attack_msg & msg)
{
	msg.target_faction = 0;
	if(_immune)
	{
		msg._attack_state |= gactive_imp::AT_STATE_IMMUNE;
	}
}       

void 
invincible_filter::TranslateEnchant(const XID & attacker,enchant_msg & msg)
{
	if(!msg.helpful)
	{
		msg.target_faction = 0;
		if(_immune) msg._attack_state |= gactive_imp::AT_STATE_IMMUNE;
	}
}       

void 
invincible_filter::Heartbeat(int tick)
{
	if(_timeout < 0) return;
	if(--_timeout <= 0) 
	{
		_is_deleted = true;
	}
}

void 
invincible_filter_2::OnAttach()
{
	gobject * pObj = _parent.GetImpl()->_parent; 
	if( (_isAggressive = pObj->msg_mask & gobject::MSG_MASK_PLAYER_MOVE))
	{
		pObj->msg_mask &= ~gobject::MSG_MASK_PLAYER_MOVE;
	}
	if(!_parent.IsPlayerClass())
	{
		_parent.ClearSpecFilter(filter::FILTER_MASK_DEBUFF);
	}
}

void 
invincible_filter_2::OnRelease()
{
	if(_isAggressive)
	{
		gobject * pObj = _parent.GetImpl()->_parent; 
		pObj->msg_mask |= gobject::MSG_MASK_PLAYER_MOVE;
	}
}

void 
invincible_filter_to_spec_id::TranslateRecvAttack(const XID & attacker,attack_msg & msg)
{
	if(attacker == _who)
	{
		msg.target_faction = 0;
		msg.force_attack  = 0;
	}
}       

void 
invincible_filter_to_spec_id::TranslateEnchant(const XID & attacker,enchant_msg & msg)
{
	if(attacker == _who)
	{
		msg.target_faction = 0;
		msg.force_attack = 0; 
	}
}       


void 
invincible_banish_filter::OnAttach()
{
	if(!_parent.IsPlayerClass())
	{
		_parent.ClearSpecFilter(filter::FILTER_MASK_DEBUFF);
	}
	_parent.IncVisibleState(49);
}

void 
invincible_banish_filter::OnRelease()
{
	_parent.DecVisibleState(49);
}

void 
invincible_banish_filter::TranslateRecvAttack(const XID & attacker,attack_msg & msg)
{
	msg.target_faction = 0;
	msg.force_attack  = 0;
}       

void 
invincible_banish_filter::TranslateEnchant(const XID & attacker,enchant_msg & msg)
{
	msg.target_faction = 0;
	msg.force_attack = 0; 
	msg.helpful = false;
}       

void 
invincible_banish_filter::Heartbeat(int tick)
{
	if(_timeout < 0) return;
	if(--_timeout <= 0) 
	{
		_is_deleted = true;
	}
}


