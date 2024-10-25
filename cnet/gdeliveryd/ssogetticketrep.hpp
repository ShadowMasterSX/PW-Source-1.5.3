
#ifndef __GNET_SSOGETTICKETREP_HPP
#define __GNET_SSOGETTICKETREP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "ssouser"
#include "mapuser.h"
#include "ssogetticket_re.hpp"
#include "gdeliveryserver.hpp"

namespace GNET
{

class SSOGetTicketRep : public GNET::Protocol
{
	#include "ssogetticketrep"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * userinfo = UserContainer::GetInstance().FindUser(user.userid);
		if(userinfo == NULL)
			return;

		SSOGetTicket_Re re(retcode, ticket, local_context, userinfo->localsid);
		GDeliveryServer::GetInstance()->Send(userinfo->linksid,re);
	}
};

};

#endif
