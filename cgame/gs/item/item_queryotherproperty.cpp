#include "../clstab.h"
#include "../world.h"
#include "../player_imp.h"
#include "../cooldowncfg.h"
#include "item_queryotherproperty.h"

DEFINE_SUBSTANCE(queryotherproperty_item, item_body, CLS_ITEM_QUERYOTHERPROPERTY)		//CLS在clstab.h中定义

int queryotherproperty_item::OnUseWithTarget(item::LOCATION l,int index,gactive_imp * obj,const XID & target, char force_attack)
{
	gplayer_imp * pImp = (gplayer_imp*)obj;
	gplayer * pPlayer = (gplayer*)(pImp->_parent);
	//物品使用冷却
	if(!pImp->CheckCoolDown(COOLDOWN_INDEX_QUERY_OTHER_PROPERTY)) 
	{
		pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}
	if(!target.IsPlayer() || target.id == pPlayer->ID.id)
		return -1;
	
	world::object_info info;
	if(!pImp->_plane->QueryObject(target,info)) return -1;
	//A3DVECTOR pos = pPlayer->pos;
	//float dis = pos.squared_distance(info.pos);
	//if(dis > 100.f*100.f) return -1;
	
	pImp->SendTo<0>(GM_MSG_QUERY_PROPERTY,target,
			index,NULL,0);

	pImp->SetCoolDown(COOLDOWN_INDEX_QUERY_OTHER_PROPERTY,QUERY_OTHER_PROPERTY_COOLDOWN_TIME);
	return 0;	//物品在player接收到GM_MSG_QUERY_PROPERTY_REPLY时扣除
}
