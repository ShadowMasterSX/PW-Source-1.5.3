
#ifndef __GNET_TOUCHPOINTQUERY_HPP
#define __GNET_TOUCHPOINTQUERY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "game2au.hpp"
#include "gproviderserver.hpp"
#include "touchpointquery_re.hpp"

#include "mapuser.h"

namespace GNET
{

class TouchPointQuery : public GNET::Protocol
{
	#include "touchpointquery"
	void SendResult(int sid)
	{		
		GProviderServer::GetInstance()->DispatchProtocol(
				GProviderServer::GetInstance()->FindGameServer(sid),
				TouchPointQuery_Re(roleid,0,0));
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole(roleid);

		if (NULL==pinfo)
		{
			SendResult(sid);
			Log::log(LOG_ERR, "role%d touch query game2au fail",roleid);
			return;
		}

		Game2AU proto;
		proto.userid = pinfo->userid;
		proto.qtype = Game2AU::QUERY_TOUCH_POINT;
		
		if(!GAuthClient::GetInstance()->SendProtocol( proto ))
		{
			Log::log(LOG_ERR, "role%d touch query game2au send fail",roleid);
		}
	
		proto.qtype = Game2AU::QUERY_ADDUP_MONEY;
		
		if(!GAuthClient::GetInstance()->SendProtocol( proto ))
		{
			Log::log(LOG_ERR, "role%d addmoney query game2au send fail",roleid);
		}
	}
};

};

#endif
