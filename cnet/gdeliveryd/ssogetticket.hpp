
#ifndef __GNET_SSOGETTICKET_HPP
#define __GNET_SSOGETTICKET_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "ssouser"
#include "mapuser.h"
#include "ssogetticketreq.hpp"
#include "gauthclient.hpp"

namespace GNET
{

class SSOGetTicket : public GNET::Protocol
{
	#include "ssogetticket"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * userinfo = UserContainer::GetInstance().FindUser(user.userid);
		if(userinfo == NULL)
			return;

		int need_extra_info = SSO_GET_TICKET_EXTRA_INFO_NONE;
		try{ Marshal::OctetsStream(info) >> need_extra_info; }catch(Marshal::Exception){}

		switch(need_extra_info)
		{
			case SSO_GET_TICKET_EXTRA_INFO1:
			{
				int zoneid = (unsigned char)GDeliveryServer::GetInstance()->zoneid;
				Marshal::OctetsStream os;
				os << zoneid;
				this->info.insert(this->info.end(), os.begin(), os.size());
			}
			break;
				
			default:
				break;
		}

		SSOGetTicketReq req(user, userinfo->ip, toaid, tozoneid, info, local_context, Octets());
		GAuthClient::GetInstance()->SendProtocol(req);
	}
};

};

#endif
