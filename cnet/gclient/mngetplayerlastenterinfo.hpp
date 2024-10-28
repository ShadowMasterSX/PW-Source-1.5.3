
#ifndef __GNET_MNGETPLAYERLASTENTERINFO_HPP
#define __GNET_MNGETPLAYERLASTENTERINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNGetPlayerLastEnterInfo : public GNET::Protocol
{
	#include "mngetplayerlastenterinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
