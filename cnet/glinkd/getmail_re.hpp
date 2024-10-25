
#ifndef __GNET_GETMAIL_RE_HPP
#define __GNET_GETMAIL_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "gmail"
namespace GNET
{

class GetMail_Re : public GNET::Protocol
{
	#include "getmail_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
