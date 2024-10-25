#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../item_list.h"
#include "item_rune.h"
#include "../clstab.h"
#include "../rune_filter.h"
#include "../cooldowncfg.h"

//DEFINE_SUBSTANCE(item_physic_rune,item_body,CLS_ITEM_PHYSIC_RUNE)
//DEFINE_SUBSTANCE(item_magic_rune,item_body,CLS_ITEM_MAGIC_RUNE)
DEFINE_SUBSTANCE(item_damage_rune,item_body,CLS_ITEM_DAMAGE_RUNE)
DEFINE_SUBSTANCE(item_defense_rune,item_body,CLS_ITEM_DEFENSE_RUNE)
DEFINE_SUBSTANCE(item_resistance_rune,item_body,CLS_ITEM_RESISTANCE_RUNE)

bool
item_damage_rune::VerifyRequirement(item_list & list,gactive_imp* obj)
{
	short l = obj->_cur_item.weapon_level;
	return (l <= _ess.require_weapon_level_max && l >= _ess.require_weapon_level_min);
}

void 
item_damage_rune::OnTakeOut(item::LOCATION l,size_t pos, size_t count, gactive_imp* obj)
{
	if( l == item::BODY)
	{
		Deactivate(l,pos,count,obj); 
	}
}

void 
item_damage_rune::OnActivate(item::LOCATION ,size_t pos, size_t count, gactive_imp* obj)
{
	obj->_cur_rune.rune_type  = _ess.rune_type;
	obj->_cur_rune.rune_level_min = _ess.require_weapon_level_min;
	obj->_cur_rune.rune_level_max = _ess.require_weapon_level_max;
	obj->_cur_rune.rune_enhance  = _ess.enhance_damage;
}

void 
item_damage_rune::OnDeactivate(item::LOCATION ,size_t pos, size_t count,gactive_imp*obj)
{
	obj->_cur_rune.rune_type = -1;
	obj->_cur_rune.rune_level_min = 0;
	obj->_cur_rune.rune_level_max = 0;
	obj->_cur_rune.rune_enhance = 0;
}

/*
int 
item_physic_rune::OnUse(item::LOCATION ,gactive_imp * obj,size_t count)
{
	short l = obj->_cur_item.weapon_level;
	if(l > _ess.require_weapon_level_max || l < _ess.require_weapon_level_min) return -1;
	if(obj->_cur_rune.physic_rune_level_min >= _ess.require_weapon_level_min)
	{
		//无法使用物品
		return -1;
	}

	obj->_cur_rune.physic_rune_level_min = _ess.require_weapon_level_min;
	obj->_cur_rune.physic_rune_level_max = _ess.require_weapon_level_max;
	obj->_cur_rune.physic_rune_enhance = _ess.enhance_damage;
	obj->_runner->use_item(_tid);
	return 1;
}

int 
item_magic_rune::OnUse(item::LOCATION ,gactive_imp * obj,size_t count)
{
	short l = obj->_cur_item.weapon_level;
	if(l > _ess.require_weapon_level_max || l < _ess.require_weapon_level_min) return -1;
	if(obj->_cur_rune.magic_rune_level_min >= _ess.require_weapon_level_min)
	{
		//无法使用物品
		return -1;
	}

	obj->_cur_rune.magic_rune_level_min = _ess.require_weapon_level_min;
	obj->_cur_rune.magic_rune_level_max = _ess.require_weapon_level_max;
	obj->_cur_rune.magic_rune_enhance = _ess.enhance_damage;
	obj->_runner->use_item(_tid);
	return 1;
}
*/


int 
item_defense_rune::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	if(obj->GetHistoricalMaxLevel() > _ess.require_player_level)
	{
		obj->_runner->error_message(S2C::ERR_LEVEL_NOT_MATCH);
		return -1;
	}
	if(obj->_filters.IsFilterExist(FILTER_DEFENSE_RUNE))
	{
		obj->_runner->error_message(S2C::ERR_RUNE_IS_IN_EFFECT);
		return -1;
	}

	if(!obj->CheckCoolDown(COOLDOWN_INDEX_DEF_RUNE))
	{
		obj->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}
	
	obj->SetCoolDown(COOLDOWN_INDEX_DEF_RUNE,8000);
	

	obj->_filters.AddFilter(new defense_rune_filter(obj,FILTER_DEFENSE_RUNE,_ess.reduce_times,1.0f - _ess.enhance_percent));
	obj->_runner->use_item(_tid);
	return 1;
}

int 
item_resistance_rune::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	if(obj->GetHistoricalMaxLevel() > _ess.require_player_level)
	{
		obj->_runner->error_message(S2C::ERR_LEVEL_NOT_MATCH);
		return -1;
	}

	if(obj->_filters.IsFilterExist(FILTER_RESISTANCE_RUNE))
	{
		obj->_runner->error_message(S2C::ERR_RUNE_IS_IN_EFFECT);
		return -1;
	}

	if(!obj->CheckCoolDown(COOLDOWN_INDEX_DEF_RUNE))
	{
		obj->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}
	
	obj->SetCoolDown(COOLDOWN_INDEX_DEF_RUNE,8000);

	obj->_filters.AddFilter(new resistance_rune_filter(obj,FILTER_RESISTANCE_RUNE,_ess.reduce_times,1.0f - _ess.enhance_percent));
	obj->_runner->use_item(_tid);
	return 1;
}



