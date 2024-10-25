#include "../world.h"
#include "item_petfood.h"
#include "../clstab.h"
#include "../cooldowncfg.h"

DEFINE_SUBSTANCE(item_pet_food,item_body,CLS_ITEM_PET_FOOD)

int
item_pet_food::OnUse(item::LOCATION l,gactive_imp * pImp,size_t count)
{
	if(!pImp->CheckCoolDown(COOLDOWN_INDEX_FEED_PET)) 
	{
		pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}
	if(pImp->FeedPet(_ess.food_type, _ess.honor))
	{
		pImp->SetCoolDown(COOLDOWN_INDEX_FEED_PET,PET_FOOD_COOLDOWN_TIME);
		return 1;
	}
	return 0;
}

