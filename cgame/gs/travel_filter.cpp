#include "string.h"
#include "travel_filter.h"
#include "sfilterdef.h"
#include "clstab.h"
#include "world.h"
#include "player_imp.h"

DEFINE_SUBSTANCE(travel_filter,filter,CLS_FILTER_TRAVEL_MODE)
void 
travel_filter::OnAttach()
{
	bool bRst;
	gplayer_imp * pImp = (gplayer_imp*)_parent.GetImpl();
	bRst = pImp->StartTravelSession(_min_time,_max_time,_target,_speed,_vehicle);
	if(bRst)
	{
		pImp->_runner->player_start_travel(_line_no,_target,_speed,_vehicle);
	}
	else
	{
		ASSERT(false);
		_is_deleted = true;
	}
}

void 
travel_filter::OnRelease()
{
	gplayer_imp * pImp = (gplayer_imp*)_parent.GetImpl();
	pImp->CompleteTravel(_vehicle,_target);
	pImp->_runner->player_complete_travel(_vehicle);
}

void 
travel_filter::OnModify(int ctrlname,void * ctrlval,size_t ctrllen)
{
	if(ctrlname == FMID_COMPLETE_TRAVEL && _min_time <= 0)
	{
		_is_deleted = true;
	}
}

void 
travel_filter::Heartbeat(int tick)
{
	_min_time --;
	_max_time --;
	if(_max_time < 0)
	{
		_is_deleted = true;	
	}
}

void 
travel_filter::TranslateSendAttack(const XID & target,attack_msg & msg)
{
	msg.target_faction = 0;
	msg.force_attack  = 0;
}

void 
travel_filter::TranslateRecvAttack(const XID & attacker,attack_msg & msg)
{
	msg.target_faction = 0;
	msg.force_attack  = 0;
}

void 
travel_filter::TranslateEnchant(const XID & attacker,enchant_msg & msg)
{
	msg.target_faction = 0;
	msg.force_attack = 0; 
}

