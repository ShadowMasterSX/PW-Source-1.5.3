
#ifndef __GNET_CHATROOMINVITE_RE_HPP
#define __GNET_CHATROOMINVITE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ChatRoomInvite_Re : public GNET::Protocol
{
	#include "chatroominvite_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryClient* dcm = GDeliveryClient::GetInstance();
		GLinkServer*     lsm = GLinkServer::GetInstance();
		if(manager==dcm)
		{
			unsigned int id = localsid;
			this->localsid = 0;
			GLinkServer::GetInstance()->Send(id,this);	
		}else{
			if (!GLinkServer::ValidRole(sid,invitee))
			{
				lsm->SessionError(sid,ERR_INVALID_ACCOUNT,"Error userid or roleid.");
				return;
			}	
			dcm->SendProtocol(this);
		}
	}
};

};

#endif
