
#ifndef __GNET_SETHELPSTATES_RE_HPP
#define __GNET_SETHELPSTATES_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class SetHelpStates_Re : public GNET::Protocol
{
	#include "sethelpstates_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		unsigned int tmplocalsid=localsid;
		this->localsid=_SID_INVALID;
		GLinkServer::GetInstance()->Send(tmplocalsid,this);

	}
};

};

#endif
