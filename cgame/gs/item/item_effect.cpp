#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../item_list.h"
#include "item_effect.h"
#include "../clstab.h"
#include "../effect_filter.h"
#include "../cooldowncfg.h"

DEFINE_SUBSTANCE(item_facepill,item_body,CLS_ITEM_FACEPILL)



int 
item_base_effect::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	int filterid = GetFilterID();
	int t = GetTimeOut();
	if(t > 0)
	{
		int cd_idx = 0;
		int cd = GetCoolDown(cd_idx);
		if(cd && !obj->CheckCoolDown(cd_idx)) return -1;
		if(cd) obj->SetCoolDown(cd_idx,cd);
		obj->_filters.AddFilter(new player_effect_filter(obj,filterid, t,_tid & 0xFFFF));
	}
	else
	{
		if(!obj->_filters.IsFilterExist(filterid))
		{
			obj->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
			return -1;
		}
		obj->_filters.RemoveFilter(filterid);
	}
	return 1;
}

int 
item_facepill::GetFilterID()
{
	return FILTER_INDEX_FACEPILL;
}

int 
item_facepill::GetCoolDown(int & cd_idx)
{
	cd_idx = COOLDOWN_INDEX_FACEPILL;
	return FACEPILL_COOLDOWN_TIME;
}

int 
item_facepill::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	if(!((1<<(obj->GetObjectClass() & 0x1F)) & _ess.require_class)) 
	{
		obj->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
		return -1;
	}
	return item_base_effect::OnUse(l,obj,count);
}

