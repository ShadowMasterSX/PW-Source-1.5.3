#include "../clstab.h"
#include "../world.h"
#include "../worldmanager.h"
#include "../player_imp.h"
#include "item_sharpener.h"

DEFINE_SUBSTANCE(sharpener_item,item_body, CLS_ITEM_SHARPENER)

int 
sharpener_item::OnUse(item::LOCATION l, int index, gactive_imp * imp, const char * arg, size_t arg_size)
{
	ASSERT(arg_size == sizeof(unsigned int));
	
	DATA_TYPE dt;
	SHARPENER_ESSENCE * ess = (SHARPENER_ESSENCE *)world_manager::GetDataMan().get_data_ptr(_tid,ID_SPACE_ESSENCE,dt);
	if(dt != DT_SHARPENER_ESSENCE || ess == NULL) return -1;

	unsigned int equip_idx = *(unsigned int*)arg;
	if(equip_idx >= item::EQUIP_INVENTORY_COUNT || !((1ULL<<equip_idx)&ess->equip_mask)) return -1;
	
	if(ess->addon_time <= 0) return -1;
	int expire_time = time(NULL) + ess->addon_time;
	
	addon_data addon_list[3];
	size_t i = 0;
	for( ; i<3 && i<sizeof(ess->addon)/sizeof(int); i++)
	{
		if(ess->addon[i] <= 0) break;
		if(!world_manager::GetDataMan().generate_addon(ess->addon[i],addon_list[i])) return -1; 
		if(addon_list[i].arg[1] != 0xFFFF) return -1; //必须是时效附加属性	
		addon_list[i].arg[1] = expire_time;
	}
	if(i == 0) return -1;
	
	if(!((gplayer_imp*)imp)->SharpenEquipment(equip_idx,addon_list,i,ess->level,ess->gfx_index))
		return -1;
	
	return 1;
}

