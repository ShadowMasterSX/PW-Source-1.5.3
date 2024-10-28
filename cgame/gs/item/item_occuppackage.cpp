#include "item_occuppackage.h"
#include "../clstab.h"
#include "../cooldowncfg.h"
#include "../world.h"
#include "../player_imp.h"

DEFINE_SUBSTANCE(item_occup_package,item_body,CLS_ITEM_OCCUP_PACKAGE)

int
item_occup_package::OnUse(item::LOCATION l,gactive_imp * imp,size_t count)
{
	int cls;
	bool gender;
	gplayer_imp* pImp = (gplayer_imp*)imp;
	pImp->GetPlayerClass(cls,gender);	

	item_list & inv = pImp->GetInventory();
	if(inv.IsFull()) return -1;

	DATA_TYPE dt;
	const ITEM_PACKAGE_BY_PROFESSION_ESSENCE & ess = *(const ITEM_PACKAGE_BY_PROFESSION_ESSENCE *)world_manager::GetDataMan().get_data_ptr(_tid, ID_SPACE_ESSENCE, dt);
	ASSERT(&ess || dt == DT_ITEM_PACKAGE_BY_PROFESSION_ESSENCE);
	int itemid = ess.prof_list[cls].gender_list[gender?1:0].item_id;

	element_data::item_tag_t tag = {element_data::IMT_NULL, 0};
	item_data * data = world_manager::GetDataMan().generate_item_from_player(itemid, &tag, sizeof(tag));
	if(!data) return -1;
	
	int rst = inv.PushInEmpty(0, *data, 1);
	if(rst >= 0)
	{
		inv[rst].AfterUnpackage(imp);
	
		std::string ITEM1;
		inv[rst].DumpDetail(ITEM1);

		GLog::formatlog("occuppackage use: roleid:%d unpackage %s",
			pImp->_parent->ID.id, ITEM1.c_str());

		pImp->_runner->obtain_item(data->type, 0, 1, inv[rst].count, gplayer_imp::IL_INVENTORY, rst);
		pImp->FirstAcquireItem(data);
	}
	else
	{
		ASSERT(false);
		FreeItem(data);
		return -1;
	}

	FreeItem(data);
	return 1;
}

