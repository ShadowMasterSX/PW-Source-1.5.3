
#ifndef __GNET_DOMAINCMD_RE_HPP
#define __GNET_DOMAINCMD_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class DomainCmd_Re : public GNET::Protocol
{
	#include "domaincmd_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
