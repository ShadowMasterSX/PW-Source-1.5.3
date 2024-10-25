
#ifndef __GNET_ROLEFORBIDUPDATE_HPP
#define __GNET_ROLEFORBIDUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "roleforbidbean"


namespace GNET
{

class RoleForbidUpdate : public GNET::Protocol
{
	#include "roleforbidupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
