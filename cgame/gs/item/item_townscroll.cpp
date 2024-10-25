#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../item_list.h"
#include "item_townscroll.h"
#include "../clstab.h"
#include "../playertemplate.h"
#include "../player_imp.h"
#include "../cooldowncfg.h"
#include <arandomgen.h>

DEFINE_SUBSTANCE(townscroll_item,item_body,CLS_ITEM_TOWNSCROLL)
DEFINE_SUBSTANCE(townscroll2_item,item_body,CLS_ITEM_TOWNSCROLL2)

int
townscroll_item::OnUse(item::LOCATION l,gactive_imp * imp,size_t count)
{
	gplayer_imp * pImp = (gplayer_imp * )imp;
	if(pImp->GetPlayerState() == gplayer_imp::PLAYER_STATE_BIND)
	{
		pImp->_runner->error_message(S2C::ERR_USE_ITEM_FAILED);
		return -1;
	}
	
	if(!pImp->CheckCoolDown(COOLDOWN_INDEX_TOWN_SCROLL))
	{
		pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}
	pImp->SetCoolDown(COOLDOWN_INDEX_TOWN_SCROLL,TOWN_SCROLL_COOLDOWN_TIME);
	//开始使用		
	if(((gplayer_imp*)pImp)->ReturnToTown())
	{
		return 1;
	}
	pImp->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
	return -1;
}

int     
townscroll2_item::OnUse(item::LOCATION , int, gactive_imp* imp,const char * arg, size_t arg_size)
{       
	if(arg_size != sizeof(int)) return -1;
	gplayer_imp * pImp = (gplayer_imp *)imp;
	if(pImp->GetPlayerState() == gplayer_imp::PLAYER_STATE_BIND)
	{
		pImp->_runner->error_message(S2C::ERR_USE_ITEM_FAILED);
		return -1;
	}

	if(pImp->IsCombatState())
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_OPERATION_IN_COMBAT);
		return -1;
	}
	if(!pImp->CheckCoolDown(COOLDOWN_INDEX_TOWN_SCROLL))
	{
		pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}

	int waypoint = (*(int*)arg) & 0xFFFF;
	if(!pImp->IsWaypointActived(waypoint) || !pImp->ReturnWaypoint(waypoint))
	{
		pImp->_runner->error_message(S2C::ERR_USE_ITEM_FAILED);
		return -1;
	}

	pImp->SetCoolDown(COOLDOWN_INDEX_TOWN_SCROLL,TOWN_SCROLL_COOLDOWN_TIME);
	return 1;
}

