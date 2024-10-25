
#ifndef __GNET_FACTIONRENAMEGSVERIFY_HPP
#define __GNET_FACTIONRENAMEGSVERIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class FactionRenameGsVerify : public GNET::Protocol
{
	#include "factionrenamegsverify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
