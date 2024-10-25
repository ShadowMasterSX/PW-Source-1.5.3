
#ifndef __GNET_STOCKTRANSACTION_HPP
#define __GNET_STOCKTRANSACTION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmailsyncdata"

namespace GNET
{

class StockTransaction : public GNET::Protocol
{
	#include "stocktransaction"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
