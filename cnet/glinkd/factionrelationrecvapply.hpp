
#ifndef __GNET_FACTIONRELATIONRECVAPPLY_HPP
#define __GNET_FACTIONRELATIONRECVAPPLY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class FactionRelationRecvApply : public GNET::Protocol
{
	#include "factionrelationrecvapply"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
