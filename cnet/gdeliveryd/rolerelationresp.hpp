
#ifndef __GNET_ROLERELATIONRESP_HPP
#define __GNET_ROLERELATIONRESP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "rolebean"
#include "factionidbean"
#include "rolegroupbean"
#include "roleinfobean"

namespace GNET
{

class RoleRelationResp : public GNET::Protocol
{
	#include "rolerelationresp"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
