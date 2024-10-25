#include "string.h"
#include "world.h"
#include "towerbuild_filter.h"
#include "clstab.h"
#include "actobject.h"

DEFINE_SUBSTANCE(towerbuild_filter,filter, CLS_FILTER_TOWER_BUILD)

void 
towerbuild_filter::AdjustDamage(damage_entry & ent, const XID &, const attack_msg&, float)
{
}

void 
towerbuild_filter::Heartbeat(int tick)
{
	_time --;
	if(_time < 0)
	{
		gactive_imp * pImp = (gactive_imp*)_parent.GetImpl();
		pImp->SendTo<0>(GM_MSG_NPC_TRANSFORM2,_parent.GetSelfID(),_final_form);
		_time = 3;
	}
}

