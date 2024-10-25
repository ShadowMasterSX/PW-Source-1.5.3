
#ifndef __GNET_INCREASEFORCEACTIVITY_HPP
#define __GNET_INCREASEFORCEACTIVITY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class IncreaseForceActivity : public GNET::Protocol
{
	#include "increaseforceactivity"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
