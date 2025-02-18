
#ifndef __GNET_CASHPASSWORDSET_RE_HPP
#define __GNET_CASHPASSWORDSET_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class CashPasswordSet_Re : public GNET::Protocol
{
	#include "cashpasswordset_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
