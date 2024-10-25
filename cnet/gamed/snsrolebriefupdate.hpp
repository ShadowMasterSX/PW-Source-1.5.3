
#ifndef __GNET_SNSROLEBRIEFUPDATE_HPP
#define __GNET_SNSROLEBRIEFUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "snsrolebrief"
#include "snsroleskills"
#include "snsroleequipment"
#include "snsrolepetcorral"

namespace GNET
{

class SNSRoleBriefUpdate : public GNET::Protocol
{
	#include "snsrolebriefupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
