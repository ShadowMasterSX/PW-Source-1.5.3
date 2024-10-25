
#ifndef __GNET_USERADDCASH_HPP
#define __GNET_USERADDCASH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class UserAddCash : public GNET::Protocol
{
	#include "useraddcash"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (!GLinkServer::GetInstance()->ValidUser(sid,userid)) return;
		this->localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
