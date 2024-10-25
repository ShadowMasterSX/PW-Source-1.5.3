#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../item_list.h"
#include "item_mobgen.h"
#include "../clstab.h"
#include "../cooldowncfg.h"
#include "../obj_interface.h"

DEFINE_SUBSTANCE(item_mob_gen,item_body,CLS_ITEM_MOBGEN)

int 
item_mob_gen::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	if(!obj->_commander->HasGMPrivilege())
	{
		obj->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
		return -1;
	}
	
	if(!obj->CheckCoolDown(COOLDOWN_INDEX_MOV_GEN))
	{
		obj->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return 0;
	}
	obj->SetCoolDown(COOLDOWN_INDEX_MOV_GEN,ITEM_MOBGEN_COOLDOWN_TIME);

	//检测是否矿物或者怪物还要..........$$$$$$$$$$$$$$$$
	
	object_interface::minor_param param;
	memset(&param,0,sizeof(param));
	param.mob_id = _mob_id;
	param.exp_factor = 1.0f;
	param.sp_factor = 1.0f;
	param.drop_rate = 1.f;
	param.money_scale = 1.f;
	param.spec_leader_id = XID(-1,-1);
	param.parent_is_leader = false;
	param.use_parent_faction = false;
	//param.die_with_leader = false;
	param.die_with_who = 0x00;
	object_interface oi(obj);
	oi.CreateMinors(param,5.f);
	return 1;
}


