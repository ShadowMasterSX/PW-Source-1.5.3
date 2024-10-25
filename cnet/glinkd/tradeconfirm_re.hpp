
#ifndef __GNET_TRADECONFIRM_RE_HPP
#define __GNET_TRADECONFIRM_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class TradeConfirm_Re : public GNET::Protocol
{
	#include "tradeconfirm_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
