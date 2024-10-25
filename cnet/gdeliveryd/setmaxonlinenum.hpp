
#ifndef __GNET_SETMAXONLINENUM_HPP
#define __GNET_SETMAXONLINENUM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gdeliveryserver.hpp"
#include "mapuser.h"
namespace GNET
{

class SetMaxOnlineNum : public GNET::Protocol
{
	#include "setmaxonlinenum"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("gdelivery:setmaxonlinenum: real num %d,fake num %d\n",maxnum,fake_maxnum);
		UserContainer::GetInstance().SetPlayerLimit( maxnum,fake_maxnum );
		GDeliveryServer::GetInstance()->Send( sid, SetMaxOnlineNum_Re((int)ERR_SUCCESS) );
	}
};

};

#endif
