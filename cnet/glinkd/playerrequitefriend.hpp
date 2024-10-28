
#ifndef __GNET_PLAYERREQUITEFRIEND_HPP
#define __GNET_PLAYERREQUITEFRIEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PlayerRequiteFriend : public GNET::Protocol
{
	#include "playerrequitefriend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkServer* lsm=GLinkServer::GetInstance();
		if ( !lsm->ValidRole(sid,roleid))
			return;
		
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
