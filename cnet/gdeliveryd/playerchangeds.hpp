
#ifndef __GNET_PLAYERCHANGEDS_HPP
#define __GNET_PLAYERCHANGEDS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PlayerChangeDS : public GNET::Protocol
{
	#include "playerchangeds"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
