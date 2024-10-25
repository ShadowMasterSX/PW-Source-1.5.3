
#ifndef __GNET_TRADEEND_HPP
#define __GNET_TRADEEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class TradeEnd : public GNET::Protocol
{
	#include "tradeend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (cause == _TRADE_END_NORMAL)
			DEBUG_PRINT("\nTransaction end normally.\n");
		if (cause == _TRADE_END_TIMEOUT)
			DEBUG_PRINT("\nTransaction end because of timeout.\n");
	}
};

};

#endif
