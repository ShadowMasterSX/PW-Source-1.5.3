
#ifndef __GNET_WEBGETROLELIST_RE_HPP
#define __GNET_WEBGETROLELIST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "webrole"

namespace GNET
{

class WebGetRoleList_Re : public GNET::Protocol
{
	#include "webgetrolelist_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
