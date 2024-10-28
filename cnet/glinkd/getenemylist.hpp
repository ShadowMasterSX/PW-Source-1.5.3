
#ifndef __GNET_GETENEMYLIST_HPP
#define __GNET_GETENEMYLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GetEnemyList : public GNET::Protocol
{
	#include "getenemylist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
        if (!GLinkServer::ValidRole(sid, roleid))
        {
            GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
            return;
        }

        this->localsid = sid;
        GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
