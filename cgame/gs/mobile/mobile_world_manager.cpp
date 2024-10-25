#include "../world.h"
#include "../worldmanager.h"
#include "../player_imp.h"
#include "../obj_interface.h"
#include "mobile_world_manager.h"
#include <gsp_if.h>

world_message_handler * mobile_world_manager::CreateMessageHandler()
{
	return new mobile_world_message_handler(this, &_plane);
}

void mobile_world_manager::OnDeliveryConnected()
{
	GMSV::SendMobileServerRegister(GetWorldIndex(),GetWorldTag());
}

void mobile_world_manager::SetFilterWhenLogin(gplayer_imp * pImp, instance_key * )
{
	//ÉèÖÃGMÒþÉí
	object_interface obj_if(pImp);
	obj_if.SetGMInvisibleFilter(true, -1, filter::FILTER_MASK_NOSAVE);
} 


int mobile_world_message_handler::HandleMessage(world * pPlane, const MSG& msg)
{
	switch(msg.message)
	{
		case GM_MSG_PLANE_SWITCH_REQUEST:
			{
				//½ûÖ¹Íæ¼ÒÇÐ»»·þÎñÆ÷
				MSG nmsg;
				BuildMessage(nmsg,GM_MSG_ERROR_MESSAGE,msg.source,msg.target,msg.pos,S2C::ERR_CANNOT_ENTER_INSTANCE);
				_manager->SendRemotePlayerMsg(msg.source.id, nmsg);
			}
			break;
			
		default:
			return global_world_message_handler::HandleMessage(pPlane,msg); 
	}
	return 0;
}
