
#ifndef __GNET_TOUCHPOINTCOST_HPP
#define __GNET_TOUCHPOINTCOST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gtouchtrade"
#include "game2au.hpp"
#include "gproviderserver.hpp"
#include "touchpointcost_re.hpp"
#include "mapuser.h"

namespace GNET
{

class TouchPointCost : public GNET::Protocol
{
	#include "touchpointcost"
	void SendResult(int sid,int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(
				GProviderServer::GetInstance()->FindGameServer(sid),
				TouchPointCost_Re(roleid,orderid,cost,0,0,retcode));
	}
	

	void Process(Manager *manager, Manager::Session::ID sid)
	{
	// TODO
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole(roleid);

		if (NULL==pinfo)
		{
			SendResult(sid,TouchPointCost_Re::TPC_FAIL);
			Log::log(LOG_ERR, "role%d touch cost game2au fail", roleid);
			return;
		}

		Game2AU proto;
		proto.userid = pinfo->userid;
		proto.qtype = Game2AU::COST_TOUCH_POINT;
		Octets context = Marshal::OctetsStream() << roleid;
		proto.info = Marshal::OctetsStream() << orderid << cost << context << (int)0;
	
		if(!GAuthClient::GetInstance()->SendProtocol( proto ))
		{
			Log::log(LOG_ERR, "role%d touch cost game2au send fail", roleid);
		}

	}

};

};
#endif
