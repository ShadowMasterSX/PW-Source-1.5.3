
#ifndef __GNET_CHATROOMINVITE_HPP
#define __GNET_CHATROOMINVITE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ChatRoomInvite : public GNET::Protocol
{
	#include "chatroominvite"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryClient* dcm = GDeliveryClient::GetInstance();
		if(manager==dcm)
		{
			unsigned int id = localsid;
			this->localsid = 0;
			GLinkServer::GetInstance()->Send(id,this);	
		}else{
			if (!GLinkServer::ValidRole(sid,inviter))
			{
				GLinkServer::GetInstance()->SessionError(sid,ERR_INVALID_ACCOUNT,"Error userid or roleid.");
				return;
			}	
			localsid = sid;
			dcm->SendProtocol(this);
		}
	}
};

};

#endif
