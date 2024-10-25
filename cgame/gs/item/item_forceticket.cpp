
#include "../clstab.h"
#include "../world.h"
#include "../worldmanager.h"
#include "../player_imp.h"
#include "item_forceticket.h"

DEFINE_SUBSTANCE(force_ticket_item, item_body, CLS_ITEM_FORCE_TICKET)		//CLS在clstab.h中定义


void force_ticket_item::OnActivate(item::LOCATION l,size_t pos, size_t count, gactive_imp* obj)
{
	((gplayer_imp*)obj)->UpdateForceTicketInfo(ess.require_force, ess.repu_inc_ratio);
}

void force_ticket_item::OnDeactivate(item::LOCATION l,size_t pos,size_t count,gactive_imp* obj)
{
	((gplayer_imp*)obj)->UpdateForceTicketInfo(0,0);
}

int force_ticket_item::OnAutoAdjust(int& value, int max)
{
	if(ess.repu_total <= 0) return 0;
	int newvalue = (int)(value * 0.01f * (100 + ess.repu_inc_ratio) + 0.5f);
	if(newvalue > max) newvalue = max;
	int delta = newvalue - value;
	if(delta <= 0) 
	{
		return -1;
	}
	else if(delta <= ess.repu_total)
	{
		ess.repu_total -= delta;
		value = newvalue;
		return ess.repu_total;
	}
	else
	{
		value += ess.repu_total;
		ess.repu_total = 0;
		return 0;
	}
}
