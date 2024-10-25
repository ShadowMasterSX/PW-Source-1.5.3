
#ifndef __GNET_PLAYERRENAME_RE_HPP
#define __GNET_PLAYERRENAME_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"


namespace GNET
{

class PlayerRename_Re : public GNET::Protocol
{
	#include "playerrename_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer * lsm = GLinkServer::GetInstance();
		unsigned int id = localsid;
		this->localsid = 0;
		lsm->Send(id, this);
	}
};

};

#endif
