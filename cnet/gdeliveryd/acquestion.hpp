
#ifndef __GNET_ACQUESTION_HPP
#define __GNET_ACQUESTION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"

namespace GNET
{

class ACQuestion : public GNET::Protocol
{
	#include "acquestion"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("acquestion, roleid=%d", roleid);
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole( roleid );
		if ( NULL != pinfo )
			dsm->Send( pinfo->linksid, this );
	}
};

};

#endif
