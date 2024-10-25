
#ifndef __GNET_PLAYERACCUSE_HPP
#define __GNET_PLAYERACCUSE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PlayerAccuse : public GNET::Protocol
{
	#include "playeraccuse"

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
