
#ifndef __GNET_SELECTROLE_RE_HPP
#define __GNET_SELECTROLE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkclient.hpp"
#include "enterworld.hpp"
#include "vclient_if.h"

namespace GNET
{
class SelectRole_Re : public GNET::Protocol
{
	#include "selectrole_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (result==ERR_SUCCESS)
		{
			GLinkClient* cm = (GLinkClient*)manager;
			manager->ChangeState(sid,&state_GDataExchgClient);
			manager->Send(sid,EnterWorld(cm->roleid,_SID_INVALID));
			VCLIENT::AddPlayer(cm->roleid);
			DEBUG_PRINT("VCLIENT: AddPlayer %d\n",cm->roleid);
		}
	}
};

};

#endif
