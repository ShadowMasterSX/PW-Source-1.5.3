
#ifndef __GNET_ADDFRIENDREMARKS_HPP
#define __GNET_ADDFRIENDREMARKS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AddFriendRemarks : public GNET::Protocol
{
	#include "addfriendremarks"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
        if (!GLinkServer::ValidRole(sid, roleid))
        {
            GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
            return;
        }

        this->srclsid = sid;
        GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
