
#ifndef __GNET_ANNOUNCEFORBIDINFO_HPP
#define __GNET_ANNOUNCEFORBIDINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
namespace GNET
{

class AnnounceForbidInfo : public GNET::Protocol
{
	#include "announceforbidinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer *lsm = GLinkServer::GetInstance();
		lsm->Send(localsid,this);
		if (disconnect) {
			lsm->ChangeState(localsid, &state_Null);
			lsm->ActiveCloseSession( localsid );
		}
	}
};

};

#endif
