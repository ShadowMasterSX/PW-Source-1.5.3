
#ifndef __GNET_GETREMOTEROLEINFO_HPP
#define __GNET_GETREMOTEROLEINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GetRemoteRoleInfo : public GNET::Protocol
{
	#include "getremoteroleinfo"

	void Process(Manager *manager, Manager::Session::ID sid);
	//crosssystem.cpp÷–∂®“Â
};

};

#endif
