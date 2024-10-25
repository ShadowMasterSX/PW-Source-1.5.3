
#ifndef __GNET_CASHMONEYEXCHANGENOTIFY_HPP
#define __GNET_CASHMONEYEXCHANGENOTIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void notify_cash_money_exchange_rate(bool open, int rate);

namespace GNET
{

class CashMoneyExchangeNotify : public GNET::Protocol
{
	#include "cashmoneyexchangenotify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		notify_cash_money_exchange_rate(open, rate);
	}
};

};

#endif
