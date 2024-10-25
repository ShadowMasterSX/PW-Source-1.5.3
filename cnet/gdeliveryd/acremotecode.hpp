
#ifndef __GNET_ACREMOTECODE_HPP
#define __GNET_ACREMOTECODE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gdeliveryserver.hpp"
namespace GNET
{

class ACRemoteCode : public GNET::Protocol
{
	#include "acremotecode"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole( dstroleid );
		if ( NULL != pinfo )
			dsm->Send( pinfo->linksid, this );
	}
};

};

#endif
