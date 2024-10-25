#include "../clstab.h"
#include "../world.h"
#include "../worldmanager.h"
#include "../player_imp.h"
#include "item_generalcard_dice.h"

DEFINE_SUBSTANCE(generalcard_dice_item,item_body, CLS_ITEM_GENERALCARD_DICE)

int 
generalcard_dice_item::OnUse(item::LOCATION l, gactive_imp * imp, size_t count)
{
	gplayer_imp* pImp = (gplayer_imp*)imp;
	DATA_TYPE dt;
	const POKER_DICE_ESSENCE & ess = *(const POKER_DICE_ESSENCE *)world_manager::GetDataMan().get_data_ptr(_tid,ID_SPACE_ESSENCE,dt);
	ASSERT(&ess && dt == DT_POKER_DICE_ESSENCE);
	//检查包裹是否满
	item_list & inv = pImp->GetInventory();
	if(inv.IsFull()) return -1;
	//随机选择生成卡牌
	int idx = abase::RandSelect(&(ess.list[0].probability), sizeof(ess.list[0]), sizeof(ess.list)/sizeof(ess.list[0]));
	int cardid = ess.list[idx].id;
	if(cardid <= 0 || world_manager::GetDataMan().get_data_type(cardid, ID_SPACE_ESSENCE) != DT_POKER_ESSENCE)
	{
		ASSERT(false && "卡牌随机发生器模板数据错误");
		return -1;
	}
	
	element_data::item_tag_t tag = {element_data::IMT_NULL, 0};
	item_data * data = world_manager::GetDataMan().generate_item_from_player(cardid, &tag, sizeof(tag));
	if(!data) return -1;
	
	int rst = inv.PushInEmpty(0, *data, 1);
	if(rst >= 0)
	{
		pImp->_runner->obtain_item(data->type, 0, 1, inv[rst].count, gplayer_imp::IL_INVENTORY, rst);
		pImp->FirstAcquireItem(data);
		
		DATA_TYPE edt;
		const POKER_ESSENCE * pess = (const POKER_ESSENCE *)world_manager::GetDataMan().get_data_ptr(data->type, ID_SPACE_ESSENCE,edt);
		if(pess && edt == DT_POKER_ESSENCE && pess->rank >= 3) // s级以上卡牌开卡记录 
		{
			GLog::formatlog("formatlog:card_dice:userid=%d:item_id=%d:dice_id=%d",
							pImp->GetParent()->ID.id,data->type,_tid);
		}
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
