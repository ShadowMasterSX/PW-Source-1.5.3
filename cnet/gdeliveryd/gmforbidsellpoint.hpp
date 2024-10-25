
#ifndef __GNET_GMFORBIDSELLPOINT_HPP
#define __GNET_GMFORBIDSELLPOINT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "gmforbidsellpoint_re.hpp"
#include "gauthclient.hpp"
#include "forbid.hxx"
#include "mapforbid.h"
#define FOREVER           0x7fffffff
#define FORTY_EIGHT_HOURS 0//172800
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
