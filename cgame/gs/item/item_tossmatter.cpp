#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../item_list.h"
#include "item_tossmatter.h"
#include "../clstab.h"
#include "../playertemplate.h"
#include "../player_imp.h"
#include <arandomgen.h>

DEFINE_SUBSTANCE(tossmatter_item,item_body,CLS_ITEM_TOSSMATTER)

int
tossmatter_item::OnUseWithTarget(item::LOCATION l,int index,gactive_imp * obj,const XID & target, char force_attack)
{
	//����Ƿ����ʹ��
	if(obj->GetHistoricalMaxLevel() < _ess.require_level || 
			obj->_cur_prop.strength < _ess.require_strength ||
			obj->_cur_prop.agility < _ess.require_agility)
			{
				obj->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
				return -1;
			}
	//��ʼʹ��		
	//���й�������
	int damage = abase::RandNormal(_ess.damage_low,_ess.damage_high);
	if(((gplayer_imp*)obj)->ThrowDart(target,damage,_ess.attack_range,force_attack))
	{
		//�޷����䰵��
		obj->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
		return -1;
	}
	return 1;
}
