
#ifndef __GNET_ACFORBIDPROCESS_HPP
#define __GNET_ACFORBIDPROCESS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class ACForbidProcess : public GNET::Protocol
{
	#include "acforbidprocess"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
