
#ifndef __GNET_GMSETGAMEATTRI_HPP
#define __GNET_GMSETGAMEATTRI_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class GMSetGameAttri : public GNET::Protocol
{
	#include "gmsetgameattri"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
