
#ifndef __GNET_CASHMONEYEXCHANGENOTIFY_HPP
#define __GNET_CASHMONEYEXCHANGENOTIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CashMoneyExchangeNotify : public GNET::Protocol
{
	#include "cashmoneyexchangenotify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
