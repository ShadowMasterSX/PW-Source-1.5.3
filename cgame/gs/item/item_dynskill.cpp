#include "../clstab.h"
#include "../world.h"
#include "../player_imp.h"
#include "item_dynskill.h"

DEFINE_SUBSTANCE(dynskill_item, item_body, CLS_ITEM_DYNSKILL)		//CLS在clstab.h中定义

void dynskill_item::OnActivate(item::LOCATION l,size_t pos, size_t count, gactive_imp* obj)
{
	DATA_TYPE dt;
	struct DYNSKILLEQUIP_ESSENCE * ess = (struct DYNSKILLEQUIP_ESSENCE *)world_manager::GetDataMan().get_data_ptr(_tid,ID_SPACE_ESSENCE,dt);
	if(ess == NULL || dt != DT_DYNSKILLEQUIP_ESSENCE)
	{
		ASSERT(false);
		return;
	}
	ASSERT(count);
	for(size_t i=0; i<sizeof(ess->id_skill)/sizeof(ess->id_skill[0]); i++)
		if(ess->id_skill[i] > 0)
			obj->ActivateDynSkill(ess->id_skill[i],count);
}

void dynskill_item::OnDeactivate(item::LOCATION l,size_t pos,size_t count,gactive_imp* obj)
{
	DATA_TYPE dt;
	struct DYNSKILLEQUIP_ESSENCE * ess = (struct DYNSKILLEQUIP_ESSENCE *)world_manager::GetDataMan().get_data_ptr(_tid,ID_SPACE_ESSENCE,dt);
	if(ess == NULL || dt != DT_DYNSKILLEQUIP_ESSENCE)
	{
		ASSERT(false);
		return;
	}
	ASSERT(count);
	for(size_t i=0; i<sizeof(ess->id_skill)/sizeof(ess->id_skill[0]); i++)
		if(ess->id_skill[i] > 0)
			obj->DeactivateDynSkill(ess->id_skill[i],count);
}

