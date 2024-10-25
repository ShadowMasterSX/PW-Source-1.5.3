
#ifndef __GNET_ACREPORTCHEATER_HPP
#define __GNET_ACREPORTCHEATER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gproviderserver.hpp"
#include "ganticheatclient.hpp"
#include "mapuser.h"

namespace GNET
{

class ACReportCheater : public GNET::Protocol
{
	#include "acreportcheater"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GProviderServer* gpsm = GProviderServer::GetInstance();
		if( manager == (Manager*)gpsm ) // from gs
		{
			GAntiCheatClient::GetInstance()->SendProtocol(this);
		}
		else
		{
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
			if ( NULL!=pinfo)
			{
				gpsm->DispatchProtocol(pinfo->gameid, *this);
			}
		}
	}
};

};

#endif
