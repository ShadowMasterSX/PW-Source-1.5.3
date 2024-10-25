
#ifndef __GNET_REPORT2GM_HPP
#define __GNET_REPORT2GM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "report2gm_re.hpp"
namespace GNET
{

class Report2GM : public GNET::Protocol
{
	#include "report2gm"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkServer* lsm=GLinkServer::GetInstance();
		if ( !lsm->ValidRole(sid,roleid))
			return;
		
		localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
