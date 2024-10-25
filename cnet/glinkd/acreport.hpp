
#ifndef __GNET_ACREPORT_HPP
#define __GNET_ACREPORT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class ACReport : public GNET::Protocol
{
	#include "acreport"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if ( !GLinkServer::ValidRole(sid,roleid) ) 
			return;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
