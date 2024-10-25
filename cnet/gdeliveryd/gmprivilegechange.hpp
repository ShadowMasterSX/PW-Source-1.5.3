
#ifndef __GNET_GMPRIVILEGECHANGE_HPP
#define __GNET_GMPRIVILEGECHANGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gauthclient.hpp"
#include "gdeliveryserver.hpp"
#include "queryuserprivilege.hpp"
namespace GNET
{

class GMPrivilegeChange : public GNET::Protocol
{
	#include "gmprivilegechange"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* gdsm=GDeliveryServer::GetInstance();
		GAuthClient::GetInstance()->SendProtocol(QueryUserPrivilege(userid,gdsm->zoneid));
	}
};

};

#endif
