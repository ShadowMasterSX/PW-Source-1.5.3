#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../item_list.h"
#include "item_savescroll.h"
#include "../clstab.h"
#include "../playertemplate.h"
#include "../player_imp.h"
#include "../cooldowncfg.h"
#include <arandomgen.h>

DEFINE_SUBSTANCE(resurrect_scroll_item,item_body,CLS_ITEM_RESURRECT_SCROLL)

int
resurrect_scroll_item::OnUseWithTarget(item::LOCATION l,int index,gactive_imp * obj,const XID & target, char force_attack)
{
	//�������Ƿ�������player�����Ƿ��Զ
	if(target.IsPlayerClass())
	{
		obj->_runner->error_message(S2C::ERR_INVALID_TARGET);
		return -1;
	}
	world::object_info info;
	if(!obj->_plane->QueryObject(target,info)) return -1;
	if(!(info.state &  world::QUERY_OBJECT_STATE_ZOMBIE)) return -1;
	if(info.pos.squared_distance(obj->_parent->pos) > 10.f * 10.f)
	{ 
		obj->_runner->error_message(S2C::ERR_OUT_OF_RANGE);
		return -1;
	}

	//�����ȴʱ��
	if(!obj->CheckCoolDown(COOLDOWN_INDEX_RECURRECT_SCROLL)) 
	{
		obj->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
		return -1;
	}
	
	//������ȴʱ��
	obj->SetCoolDown(COOLDOWN_INDEX_RECURRECT_SCROLL,RECURRECT_SCROLL_COOLDOWN_TIME);

	//����������Ϣ
	gactive_object * pObj =  (gactive_object*) obj->_parent;
	obj->SendTo<0>(GM_MSG_SCROLL_RESURRECT,target,(pObj->object_state & gactive_object::STATE_PVPMODE)?1:0);
	return 1;
}

