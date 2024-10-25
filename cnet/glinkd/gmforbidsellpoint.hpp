
#ifndef __GNET_GMFORBIDSELLPOINT_HPP
#define __GNET_GMFORBIDSELLPOINT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"

namespace GNET
{

class GMForbidSellPoint : public GNET::Protocol
{
	#include "gmforbidsellpoint"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
	}
};

};

#endif
