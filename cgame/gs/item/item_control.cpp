#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../item_list.h"
#include "item_control.h"
#include "../playertemplate.h"
#include "../player_imp.h"
#include <arandomgen.h>

DEFINE_SUBSTANCE(item_control_mob,item_body,CLS_ITEM_CONTROLLER)

int
item_control_mob::OnUseWithTarget(item::LOCATION l,int index, gactive_imp * obj,const XID & target, char force_attack)
{
	//检查使用条件：距离等
	gplayer_imp * pImp = (gplayer_imp*)obj;
	if(int rst = pImp->CheckUseTurretScroll())
	{
		pImp->_runner->error_message(rst);
		return 0;
	}
	
	if(target.IsValid())
	{
		int faction = obj->GetFaction();
		obj->SendTo<0>(GM_MSG_BECOME_TURRET_MASTER,target,_tid,&faction,sizeof(faction));
	}
	else
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_TARGET);
		return -1;
	}
	return 0;
}

