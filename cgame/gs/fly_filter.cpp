#include "string.h"
#include "world.h"
#include "fly_filter.h"
#include "clstab.h"
#include "cooldowncfg.h"

DEFINE_SUBSTANCE(flysword_fly_filter,filter,CLS_FILTER_FLYSWORD)
DEFINE_SUBSTANCE(angel_wing_fly_filter,filter,CLS_FILTER_ANGEL_WING)

void 
flysword_fly_filter::Heartbeat(int tick)
{
	if(_is_active)
	{
		int rst = _parent.SpendFlyTime(tick);
		if(rst < 0) 
		{
		 	_is_deleted = true;
		}
		else if(rst == 0)
		{
			Deactive();
		}
	}
}

void  
flysword_fly_filter::OnModify(int ctrlname,void * ctrlval,size_t ctrllen)
{
	if(ctrlname == FMID_SPEEDUP_FLY)
	{
		_parent.SetCoolDown(COOLDOWN_INDEX_RUSH_FLY,RUSH_FLAY_COOLDOWN_TIME);
		Active();
	}
	else if (ctrlname == FMID_NORMAL_FLY)
	{
		Deactive();
	}
}

void
angel_wing_fly_filter::Heartbeat(int tick)
{
	if(!_parent.DrainMana(_mana_per_second))
	{
		_is_deleted = true; 
	}
}

bool 
angel_wing_fly_filter::Save(archive & ar)
{
	filter::Save(ar);	
	ar << _mana_per_second;
	return true;
}

bool 
angel_wing_fly_filter::Load(archive & ar)
{
	__PRINTF("load angel fly filter .................\n");
	filter::Load(ar);	
	ar >> _mana_per_second;
	return true;
}


