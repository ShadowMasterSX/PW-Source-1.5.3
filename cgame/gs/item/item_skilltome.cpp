#include "../clstab.h"
#include "string.h"
#include "../world.h"
#include "../item_list.h"
#include "item_skilltome.h"
#include "../filter.h"
#include "../fly_filter.h"
#include "../clstab.h"
#include "../playertemplate.h"
#include "../obj_interface.h"
#include "../actobject.h"
#include "../player_imp.h"

DEFINE_SUBSTANCE(skilltome_item,item_body, CLS_ITEM_SKILLTOME)
int 
skilltome_item::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	gplayer_imp * pImp = substance::DynamicCast<gplayer_imp>(obj);
	if(!pImp) return -1;
	int rlevel;
	if((rlevel = pImp->_skill.Learn(_ess.id_skill,object_interface(pImp),_ess.skill_level)) <= 0)
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
		return -1;
	}
	pImp->_runner->learn_skill(_ess.id_skill, rlevel);
	return 1;
}

bool 
skilltome_item::IsItemCanUse(item::LOCATION l)
{	
	return false;
}

