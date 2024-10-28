#include "../world.h"
#include "item_stone.h"
#include "item_addon.h"
#include "../clstab.h"
#include "../actobject.h"
#include "../item_list.h"

DEFINE_SUBSTANCE(stone_item,item_body,CLS_ITEM_STONE)

bool 
stone_item::Save(archive & ar)
{
	try
	{
		SaveAddOn(ar,_weapon_addon);
		SaveAddOn(ar,_armor_addon);
	}
	catch(...)
	{
		return false;
	}
	return true;
}

bool 
stone_item::Load(archive & ar)
{	
	//保存数据至raw_data中  $$$$以后不这样做可能
	_raw_data.clear();
	_raw_data.push_back(ar.data(),ar.size());

	try
	{
		LoadAddOn(ar,_weapon_addon);
		LoadAddOn(ar,_armor_addon);
	}
	catch(...)
	{
		return false;
	}
	ASSERT(ar.is_eof());		//由于保存数据到eof中，所以这里必须是全部数据

    UpdateEssence();
	return true;
}


void stone_item::UpdateEssence()
{
    DATA_TYPE dt;
    STONE_ESSENCE* ess = (STONE_ESSENCE*)world_manager::GetDataMan().get_data_ptr(_tid, ID_SPACE_ESSENCE, dt);
    ASSERT((ess != NULL) && (dt == DT_STONE_ESSENCE));

    addon_data data = {0};
    int addon_id = ess->id_addon_decoration;

    if ((addon_id != 0) && world_manager::GetDataMan().generate_addon(addon_id, data))
    {
        _decoration_addon.reserve(1);
        _decoration_addon.push_back(data);
    }
}


