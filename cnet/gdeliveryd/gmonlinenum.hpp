
#ifndef __GNET_GMONLINENUM_HPP
#define __GNET_GMONLINENUM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gdeliveryserver.hpp"
#include "gmonlinenum_re.hpp"
#include "mapuser.h"
namespace GNET
{

class GMOnlineNum : public GNET::Protocol
{
	#include "gmonlinenum"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		dsm->Send( sid,GMOnlineNum_Re(gmroleid,localsid,UserContainer::GetInstance().Size(),0) );
	}
};

};

#endif
