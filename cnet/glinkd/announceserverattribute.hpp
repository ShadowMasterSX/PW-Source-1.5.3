
#ifndef __GNET_ANNOUNCESERVERATTRIBUTE_HPP
#define __GNET_ANNOUNCESERVERATTRIBUTE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
namespace GNET
{

class AnnounceServerAttribute : public GNET::Protocol
{
	#include "announceserverattribute"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->SetServerAttr(attr);
		GLinkServer::GetInstance()->SetFreeCreatime(freecreatime);
		GLinkServer::GetInstance()->SetExpRate(exp_rate);
	}
};

};

#endif
