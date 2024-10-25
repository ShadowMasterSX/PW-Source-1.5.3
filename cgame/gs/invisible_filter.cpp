#include "string.h"
#include "world.h"
#include "invisible_filter.h"
#include "clstab.h"
#include "actobject.h"

DEFINE_SUBSTANCE(invisible_filter,filter,CLS_FILTER_INVISIBLE)
DEFINE_SUBSTANCE(gm_invisible_filter,filter,CLS_FILTER_GM_INVISIBLE)

void invisible_filter::Heartbeat(int tick)
{
	if(_protected_timeout > 0) _protected_timeout -= tick;
	if(_timeout > 0)
	{
		_timeout -= tick;
		if(_timeout <= 0) _is_deleted = true;
	}
	if(!_parent.DrainMana(_mana_per_second))
	{
		_is_deleted = true;
	}
}

void invisible_filter::TranslateSendAttack(const XID & target,attack_msg & msg)
{
	_is_deleted = true;		
	//马上就现身,解决隐身下打怪，怪无法反击的问题
	_parent.ClearInvisible();
}

void invisible_filter::TranslateSendEnchant(const XID & attacker,enchant_msg & msg)
{
	if(msg.helpful) return;
	_is_deleted = true;
	//马上就现身,解决隐身下打怪，怪无法反击的问题
	_parent.ClearInvisible();
}

void invisible_filter::DoEnchant(const XID & attacker,enchant_msg & msg)
{
	if(msg.helpful) return;
	if(_protected_timeout > 0)
	{
		return;
		//int invisible_degree = 0, tanti_invisible_degree = 0;
		//if(_parent.QueryObject(attacker,invisible_degree,tanti_invisible_degree) == 0) return;
		//if(_parent.GetInvisibleDegree() > tanti_invisible_degree) return;
	}
	if(msg.skill == 1907)
	{
		if(_parent.GetBasicProp().level > msg.ainfo.level) return;
	}
	_is_deleted = true;	
}

void invisible_filter::AdjustDamage(damage_entry&,  const XID & attacker, const attack_msg&,float)
{
	if(_protected_timeout > 0)
	{
		return;
		//int invisible_degree = 0, tanti_invisible_degree = 0;
		//if(_parent.QueryObject(attacker,invisible_degree,tanti_invisible_degree) == 0) return;
		//if(_parent.GetInvisibleDegree() > tanti_invisible_degree) return;
	}
	_is_deleted = true;	
}

void invisible_filter::OnAttach()
{
	_parent.IncVisibleState(58);
	_parent.InsertTeamVisibleState(149);
	_parent.SetInvisible(_invisible_degree);		
	if(_speeddown)
	{
		_parent.ImpairSpeed(_speeddown);
		_parent.ImpairSwimSpeed(_speeddown);
		_parent.ImpairScaleFlySpeed(_speeddown);
		_parent.UpdateSpeedData();
		_parent.SendClientCurSpeed();
	}
}

void invisible_filter::OnRelease()
{
	_parent.DecVisibleState(58);
	_parent.RemoveTeamVisibleState(149);
	if(_parent.IsInvisible()) _parent.ClearInvisible();		
	if(_speeddown)
	{
		_parent.EnhanceSpeed(_speeddown);
		_parent.EnhanceSwimSpeed(_speeddown);
		_parent.EnhanceScaleFlySpeed(_speeddown);
		_parent.UpdateSpeedData();
		_parent.SendClientCurSpeed();
	}
}

void gm_invisible_filter::Heartbeat(int tick)	
{
	if(_timeout > 0)
	{
		_timeout -= tick;
		if(_timeout <= 0) _is_deleted = true;
	}
}

void gm_invisible_filter::TranslateRecvAttack(const XID & attacker,attack_msg & msg)
{
	msg.target_faction = 0;
}

void gm_invisible_filter::TranslateEnchant(const XID & attacker,enchant_msg & msg)
{
	msg.target_faction = 0;
}

void gm_invisible_filter::OnAttach() 
{
	_parent.SetGMInvisible();
}

void gm_invisible_filter::OnRelease()
{
	_parent.ClearGMInvisible();
}
