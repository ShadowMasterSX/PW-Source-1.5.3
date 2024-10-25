
#ifndef __GNET_ROLESTATUSANNOUNCE_HPP
#define __GNET_ROLESTATUSANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
namespace GNET
{

class RoleStatusAnnounce : public GNET::Protocol
{
	#include "rolestatusannounce"
	//This protocol can ONLY come from gdelivery
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		unsigned int tmpsid=localsid;
		localsid=0;
		GLinkServer::GetInstance()->Send(tmpsid,this);
	}
};

};

#endif
