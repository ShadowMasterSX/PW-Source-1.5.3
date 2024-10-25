
#ifndef __GNET_TRADEREMOVEGOODS_RE_HPP
#define __GNET_TRADEREMOVEGOODS_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "groleinventory"
#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class TradeRemoveGoods_Re : public GNET::Protocol
{
	#include "traderemovegoods_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		unsigned int tmplocalsid=localsid;
		this->localsid=_SID_INVALID;
		GLinkServer::GetInstance()->Send(tmplocalsid,this);

	}
};

};

#endif
