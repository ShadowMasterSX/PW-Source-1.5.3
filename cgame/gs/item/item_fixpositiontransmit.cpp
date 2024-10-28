#include "../clstab.h"
#include "../actobject.h"
#include "item_fixpositiontransmit.h"


DEFINE_SUBSTANCE(item_fixpositiontransmit,item_body,CLS_ITEM_FIX_POSITION_TRANSMIT)
		
int 
item_fixpositiontransmit::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	return obj->AddFixPositionEnergy(_tid);
}
