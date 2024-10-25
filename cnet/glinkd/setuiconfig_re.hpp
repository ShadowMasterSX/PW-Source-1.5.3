
#ifndef __GNET_SETUICONFIG_RE_HPP
#define __GNET_SETUICONFIG_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class SetUIConfig_Re : public GNET::Protocol
{
	#include "setuiconfig_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("glinkd::receive setuiconfig_re from gdelivery.roleid=%d,retcode=%d\n",roleid,result);
		unsigned int tmplocalsid=localsid;
		this->localsid=_SID_INVALID;
		GLinkServer::GetInstance()->Send(tmplocalsid,this);

	}
};

};

#endif
