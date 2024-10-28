#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../item_list.h"
#include "../cooldowncfg.h"
#include "../player.h"
#include "item_fireworks.h"

DEFINE_SUBSTANCE(item_fireworks,item_body,CLS_ITEM_FIREWORKS)
DEFINE_SUBSTANCE(item_fireworks2,item_body,CLS_ITEM_FIREWORKS2)

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

int item_fireworks2::OnUse(item::LOCATION ,int index, gactive_imp* obj,const char * arg, size_t arg_size)
{
	if(!arg || !arg_size)
		return -1;
	if(arg_size != sizeof(_firework_args))
		return -1;
	if(!obj->CheckCoolDown(COOLDOWN_INDEX_FIREWORKS)) 
	{
		obj->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}
	
	if(((gplayer*)obj->_parent)->IsInvisible())
	{
		obj->_runner->error_message(S2C::ERR_OPERTION_DENYED_IN_INVISIBLE);
		return -1;	
	}

	obj->SetCoolDown(COOLDOWN_INDEX_FIREWORKS,FIREWORKS_COOLDOWN_TIME);

	_firework_args *firework_args = (_firework_args *)arg;
	return obj->UseFireWorks2(firework_args->is_friend_list, firework_args->target_role_id, _tid, firework_args->target_username);

	return 1;
}

