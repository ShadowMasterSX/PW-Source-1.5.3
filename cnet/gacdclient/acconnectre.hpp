
#ifndef __GNET_ACCONNECTRE_HPP
#define __GNET_ACCONNECTRE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "acquery.hpp"
#include "ganticheaterclient.hpp"

namespace GNET
{

class ACConnectRe : public GNET::Protocol
{
	#include "acconnectre"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		((GAntiCheaterClient*)manager)->SetAreaID(aid);
		printf("server %d connected!\n", aid);		
		ACQuery p;
		p.query_type = AC_QUERY_VERSION;
		GAntiCheaterClient::GetInstance()->SendProtocol(p, aid);
	}
};

};

#endif
