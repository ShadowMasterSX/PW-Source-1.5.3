
#ifndef __GNET_AUTHDVERSION_HPP
#define __GNET_AUTHDVERSION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "announceauthdversion.hpp"
#include "maplinkserver.h"

namespace GNET
{

class AuthdVersion : public GNET::Protocol
{
	#include "authdversion"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("AuthdVersion: retcode %d version %d", retcode, version);
		if(retcode == 0)
		{
			GAuthClient::GetInstance()->SetVersion(version);
			LinkServer::GetInstance().BroadcastProtocol(AnnounceAuthdVersion(version));
		}
		else
			GAuthClient::GetInstance()->IdentifyFailed();
	}
};

};

#endif
