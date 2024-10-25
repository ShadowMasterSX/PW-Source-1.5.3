#include "../clstab.h"
#include "../world.h"
#include "../worldmanager.h"
#include "../player_imp.h"
#include "item_wedding.h"
#include "../wedding_filter.h"

DEFINE_SUBSTANCE(wedding_bookcard_item, item_body, CLS_ITEM_WEDDING_BOOKCARD)		//CLS在clstab.h中定义
DEFINE_SUBSTANCE(wedding_invitecard_item, item_body, CLS_ITEM_WEDDING_INVITECARD)		//CLS在clstab.h中定义

int
wedding_invitecard_item::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	ASSERT(obj->GetRunTimeClass()->IsDerivedFrom(CLASSINFO(gplayer_imp)));
	gplayer_imp * pImp = (gplayer_imp *)obj;
	
	//if(pImp->_filters.IsFilterExist(FILTER_INDEX_WEDDING)) return -1;
	if(!world_manager::GetInstance()->WeddingCheckOngoing(ess.groom, ess.bride, ess.scene))
	{
		pImp->_runner->error_message(S2C::ERR_WEDDING_NOT_ONGOING);
		return -1;
	}
	if(ess.invitee != 0 && pImp->_parent->ID.id != ess.invitee)
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
		return -1;
	}
	
	DATA_TYPE dt;
	WEDDING_CONFIG * cfg = (WEDDING_CONFIG *)world_manager::GetDataMan().get_data_ptr(WEDDING_CONFIG_ID, ID_SPACE_CONFIG, dt);
	if(cfg == NULL || dt != DT_WEDDING_CONFIG) return -1;
	if((size_t)ess.scene >= sizeof(cfg->wedding_scene)/sizeof(cfg->wedding_scene[0])) return -1;
	float * dst_pos = cfg->wedding_scene[ess.scene].pos;
	A3DVECTOR scene_pos(dst_pos[0],dst_pos[1],dst_pos[2]);
	if(!pImp->_plane->PosInWorld(scene_pos)) return -1;
	
	pImp->LongJump(scene_pos);
	pImp->_filters.AddFilter(new wedding_filter(pImp,ess.groom,ess.bride,ess.scene));
	return 0;
}
