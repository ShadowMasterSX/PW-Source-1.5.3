#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../item_list.h"
#include "../cooldowncfg.h"
#include "../player.h"
#include "item_fireworks.h"

DEFINE_SUBSTANCE(item_fireworks,item_body,CLS_ITEM_FIREWORKS)

int
item_fireworks::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	if(!obj->CheckCoolDown(COOLDOWN_INDEX_FIREWORKS)) 
	{
		obj->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}
	obj->SetCoolDown(COOLDOWN_INDEX_FIREWORKS,FIREWORKS_COOLDOWN_TIME);
	
	if(((gplayer*)obj->_parent)->IsInvisible())
	{
		obj->_runner->error_message(S2C::ERR_OPERTION_DENYED_IN_INVISIBLE);
		return -1;	
	}
	
	return 1;
}

