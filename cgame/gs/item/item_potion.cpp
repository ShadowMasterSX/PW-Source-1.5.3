#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../item_list.h"
#include "item_potion.h"
#include "../potion_filter.h"
#include "../sfilterdef.h"
#include "../cooldowncfg.h"


DEFINE_SUBSTANCE(healing_potion,item_body,CLS_ITEM_HEALING_POTION)
DEFINE_SUBSTANCE(mana_potion,item_body,CLS_ITEM_MANA_POTION)
DEFINE_SUBSTANCE(rejuvenation_potion,item_body,CLS_ITEM_REJUVENATION_POTION)
DEFINE_SUBSTANCE(half_antidote,item_body,CLS_ITEM_HALF_ANTIDOTE)
DEFINE_SUBSTANCE(full_antidote,item_body,CLS_ITEM_FULL_ANTIDOTE)

int
healing_potion::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	if(obj->GetHistoricalMaxLevel() < _ess.require_level) 
	{
		obj->_runner->error_message(S2C::ERR_LEVEL_NOT_MATCH);
		return -1;
	}
	if(!obj->CheckCoolDown(COOLDOWN_INDEX_HP_POTION))
	{
		obj->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}
	
	obj->SetCoolDown(COOLDOWN_INDEX_HP_POTION,_ess.cool_time);
	obj->_filters.AddFilter(new healing_potion_filter(obj,_ess.time,_ess.life));
	return 1;
}

int
mana_potion::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	if(obj->GetHistoricalMaxLevel() < _ess.require_level) 
	{
		obj->_runner->error_message(S2C::ERR_LEVEL_NOT_MATCH);
		return -1;
	}
	if(!obj->CheckCoolDown(COOLDOWN_INDEX_MP_POTION))
	{
		obj->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}
	obj->SetCoolDown(COOLDOWN_INDEX_MP_POTION,_ess.cool_time);
	obj->_filters.AddFilter(new mana_potion_filter(obj,_ess.time,_ess.mana));
	return 1;
}

int
rejuvenation_potion::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	if(obj->GetHistoricalMaxLevel() < _ess.require_level) 
	{
		obj->_runner->error_message(S2C::ERR_LEVEL_NOT_MATCH);
		return -1;
	}
	if(!obj->CheckCoolDown(COOLDOWN_INDEX_REJUVENATION_POTION))
	{
		obj->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}
	obj->HealByPotion(_ess.life);
	obj->InjectMana(_ess.mana);
	obj->SetCoolDown(COOLDOWN_INDEX_REJUVENATION_POTION,_ess.cool_time);
	return 1;
}

int
half_antidote::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	if(obj->GetHistoricalMaxLevel() < _ess.require_level) 
	{
		obj->_runner->error_message(S2C::ERR_LEVEL_NOT_MATCH);
		return -1;
	}
	if(!obj->CheckCoolDown(COOLDOWN_INDEX_ANITDOTE_POTION))
	{
		obj->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}

	obj->SetCoolDown(COOLDOWN_INDEX_ANITDOTE_POTION,_ess.cool_time);
	obj->_filters.ModifyFilter(FILTER_TOXIC,FMID_ANTIDOTE,NULL,0);
	obj->_filters.ModifyFilter(FILTER_TOXIC_MERGE,FMID_ANTIDOTE,NULL,0);
	return 1;
}

int
full_antidote ::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	if(obj->GetHistoricalMaxLevel() < _ess.require_level) 
	{
		obj->_runner->error_message(S2C::ERR_LEVEL_NOT_MATCH);
		return -1;
	}
	if(!obj->CheckCoolDown(COOLDOWN_INDEX_ANITDOTE_POTION))
	{
		obj->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}

	obj->SetCoolDown(COOLDOWN_INDEX_ANITDOTE_POTION,_ess.cool_time);
	obj->_filters.RemoveFilter(FILTER_TOXIC);
	obj->_filters.RemoveFilter(FILTER_TOXIC_MERGE);
	return 1;
}

