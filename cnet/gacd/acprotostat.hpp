
#ifndef __GNET_ACPROTOSTAT_HPP
#define __GNET_ACPROTOSTAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "userdatamanager.hpp"

namespace GNET
{

class ACProtoStat : public GNET::Protocol
{
	#include "acprotostat"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		UserDataManager::GetInstance()->VisitOnline(roleid, this);
	}

	void operator() (UserSessionData *data)
	{
		data->AppendProtoStat(ProtoStat(gamedatasend, keepalive, publicchat));
	}
};

};

#endif
