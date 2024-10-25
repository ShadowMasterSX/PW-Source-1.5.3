
#ifndef __GNET_SELLPOINT_HPP
#define __GNET_SELLPOINT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "dbsellpoint.hrp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class SellPoint : public GNET::Protocol
{
	#include "sellpoint"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
	}
};

};

#endif
