
#ifndef __GNET_SSOGETTICKET_HPP
#define __GNET_SSOGETTICKET_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "ssouser"
#include "localmacro.h"

namespace GNET
{

class SSOGetTicket : public GNET::Protocol
{
	#include "ssogetticket"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (!GLinkServer::GetInstance()->ValidUser(sid,user.userid)) return;

		int need_extra_info = SSO_GET_TICKET_EXTRA_INFO_NONE;
		try{ Marshal::OctetsStream(info) >> need_extra_info; }catch(Marshal::Exception){}

		switch(need_extra_info)
		{
			default:
			case SSO_GET_TICKET_EXTRA_INFO_NONE:
				GDeliveryClient::GetInstance()->SendProtocol(this);
				break;
				
			case SSO_GET_TICKET_EXTRA_INFO1:
				SessionInfo * sinfo = GLinkServer::GetInstance()->GetSessionInfo(sid);
				if(sinfo && sinfo->roleid && sinfo->gsid != _GAMESERVER_ID_INVALID)
				{
					this->info = Marshal::OctetsStream() << need_extra_info << sinfo->roleid;
					GProviderServer::GetInstance()->DispatchProtocol(sinfo->gsid,this);	
				}
				break;
		}
	}
};

};

#endif
