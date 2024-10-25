
#ifndef __GNET_GETREMOTECNETSERVERCONFIG_RE_HPP
#define __GNET_GETREMOTECNETSERVERCONFIG_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GetRemoteCNetServerConfig_Re : public GNET::Protocol
{
	#include "getremotecnetserverconfig_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("Recv GetRemoteCNetServerConfig_Re result_size=%d, linksid=%d, localsid=%d", result.size(), linksid, localsid);
		GetCNetServerConfig_Re re(result, localsid);
		GDeliveryServer::GetInstance()->Send(linksid, re);
	}
};

};

#endif
