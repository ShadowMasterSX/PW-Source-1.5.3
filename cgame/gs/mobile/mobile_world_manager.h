#ifndef __ONLINEGAME_GS_MOBILE_WORLD_MANAGER_H__
#define __ONLINEGAME_GS_MOBILE_WORLD_MANAGER_H__

#include "../global_manager.h"

class mobile_world_manager : public global_world_manager
{

public:
	mobile_world_manager(){}
	virtual ~mobile_world_manager(){}
	virtual int GetWorldType(){ return WORLD_TYPE_MOBILESERVER; }

public:
	virtual void SetFilterWhenLogin(gplayer_imp * pImp, instance_key * );
	virtual world_message_handler * CreateMessageHandler();
	virtual void OnDeliveryConnected();
	virtual bool IsMobileWorld(){ return true; }
public:
};

class mobile_world_message_handler : public global_world_message_handler
{
public:
	mobile_world_message_handler(global_world_manager * pManager, world * plane):global_world_message_handler(pManager, plane){}
	virtual ~mobile_world_message_handler(){}

	virtual int HandleMessage(world * pPlane, const MSG& msg);
};

#endif
