
#ifndef __GNET_PLAYERNAMEUPDATE_HPP
#define __GNET_PLAYERNAMEUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"


namespace GNET
{

class PlayerNameUpdate : public GNET::Protocol
{
	#include "playernameupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer * lsm = GLinkServer::GetInstance();
		typedef std::vector<int/*localsid*/> link_localsid;
		link_localsid lls;
		lls.swap(link_localsid_list);
		for (link_localsid::iterator it = lls.begin(), et=lls.end(); it != et; ++it)
		{
			lsm->Send(*it, this);
		}
	}
};

};

#endif
