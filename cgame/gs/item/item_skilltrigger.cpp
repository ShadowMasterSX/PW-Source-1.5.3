#include "../clstab.h"
#include "../world.h"
#include "../item_list.h"
#include "item_skilltrigger.h"
#include "../obj_interface.h"
#include "../actobject.h"
#include "../cooldowncfg.h"

DEFINE_SUBSTANCE(skilltrigger_item,item_body, CLS_ITEM_SKILL_TRIGGER)

int 
skilltrigger_item::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	if(_ess.level_require > obj->GetHistoricalMaxLevel())
	{
		obj->_runner->error_message(S2C::ERR_LEVEL_NOT_MATCH);
		return -1;
	}

	if(!obj->CheckCoolDown(COOLDOWN_INDEX_SKILL_TRIGGER)) 
	{
		obj->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}

	//使用技能，激活效果
	if(!obj->CastRune(_ess.id_skill, _ess.skill_level))
	//if(!obj->CastRune(112, 3))
	{
		obj->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
		return -1;
	}
	obj->SetCoolDown(COOLDOWN_INDEX_SKILL_TRIGGER,SKILLTRIGGER_COOLDOWN_TIME);
	return 1;
}

