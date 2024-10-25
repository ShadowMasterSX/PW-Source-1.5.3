
#ifndef __GNET_SENDBATTLECHALLENGE_HPP
#define __GNET_SENDBATTLECHALLENGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gshoplog"
#include "groleinventory"
#include "gmailsyncdata"
namespace GNET
{

class SendBattleChallenge : public GNET::Protocol
{
	#include "sendbattlechallenge"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		maxbonus = Factiondb::GetInstance()->GetMaxbonus(factionid, roleid);
		if(maxbonus>0)
			GFactionServer::GetInstance()->DispatchProtocol(0, this);
	}
};

};

#endif
