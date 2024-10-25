
#ifndef __GNET_TRANSBUYPOINT_HPP
#define __GNET_TRANSBUYPOINT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "transid"
#include "sellid"
namespace GNET
{

class TransBuyPoint : public GNET::Protocol
{
	#include "transbuypoint"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
