
#ifndef __GNET_REFGETREFERENCECODE_RE_HPP
#define __GNET_REFGETREFERENCECODE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class RefGetReferenceCode_Re : public GNET::Protocol
{
	#include "refgetreferencecode_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid, this);
	}
};

};

#endif
