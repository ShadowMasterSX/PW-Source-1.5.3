
#ifndef __GNET_GETMAILLIST_RE_HPP
#define __GNET_GETMAILLIST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "gmailheader"
namespace GNET
{

class GetMailList_Re : public GNET::Protocol
{
	#include "getmaillist_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
