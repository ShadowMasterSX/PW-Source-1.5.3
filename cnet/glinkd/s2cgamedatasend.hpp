
#ifndef __GNET_S2CGAMEDATASEND_HPP
#define __GNET_S2CGAMEDATASEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gamedatasend.hpp"
#include "gdeliveryclient.hpp"
#include "statusannounce.hpp"
namespace GNET
{
class S2CGamedataSend : public GNET::Protocol
{
	#include "s2cgamedatasend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();

		if (!lsm->ValidRole(localsid,roleid))
		{
			return;
		}

		lsm->AccumulateSend(localsid,GamedataSend(data));
	}
};

};

#endif
