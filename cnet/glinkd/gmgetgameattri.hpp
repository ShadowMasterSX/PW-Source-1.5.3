
#ifndef __GNET_GMGETGAMEATTRI_HPP
#define __GNET_GMGETGAMEATTRI_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class GMGetGameAttri : public GNET::Protocol
{
	#include "gmgetgameattri"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
