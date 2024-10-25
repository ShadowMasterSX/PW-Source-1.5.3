#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../item_list.h"
#include "item_dbexp.h"
#include "../task/taskman.h"
#include "../player_imp.h"
#include "../cooldowncfg.h"

DEFINE_SUBSTANCE(item_dbl_exp,item_body,CLS_ITEM_DBL_EXP)

int
item_dbl_exp::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	ASSERT(obj->GetRunTimeClass()->IsDerivedFrom(gplayer_imp::GetClass()));
	gplayer_imp * pImp = (gplayer_imp * ) obj;
	if(!pImp->CheckCoolDown(COOLDOWN_INDEX_DBL_EXP))
	{
		pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}
	if(!pImp->ActiveDoubleExpTime(_ess.dbl_time))
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
		return -1;
	}
	pImp->SetCoolDown(COOLDOWN_INDEX_DBL_EXP,60*1000);
	return 1;
}

