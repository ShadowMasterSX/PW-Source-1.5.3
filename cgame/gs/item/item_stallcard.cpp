#include "../clstab.h"
#include "../world.h"
#include "../player_imp.h"
#include "item_stallcard.h"

DEFINE_SUBSTANCE(stallcard_item, item_body, CLS_ITEM_STALLCARD)		//CLS在clstab.h中定义

bool stallcard_item::VerifyRequirement(item_list & list, gactive_imp* imp)
{
	if(list.GetLocation() == item::BODY)
		return true;		
	else
		return false;	
}
void stallcard_item::OnPutIn(item::LOCATION l, item_list & list, size_t pos, size_t count,gactive_imp* obj)
{
	if(l == item::BODY)
	{
		DATA_TYPE dt;
		struct SELL_CERTIFICATE_ESSENCE* ess = (struct SELL_CERTIFICATE_ESSENCE*)world_manager::GetDataMan().get_data_ptr(_tid, ID_SPACE_ESSENCE, dt);
		if(ess == NULL || dt != DT_SELL_CERTIFICATE_ESSENCE)
		{
			ASSERT(false);
			return;
		}
		obj->UpdateStallInfo(_tid, ess->num_sell_item, ess->num_buy_item, ess->max_name_length*2+2);
	}
}
void stallcard_item::OnTakeOut(item::LOCATION l, size_t pos, size_t count, gactive_imp* obj)
{
	if(l == item::BODY)
	{
		obj->ClearStallInfo();
		obj->OnStallCardTakeOut();
	}
}
