
#ifndef __GNET_ACFORBIDCHEATER_HPP
#define __GNET_ACFORBIDCHEATER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class ACForbidCheater : public GNET::Protocol
{
	#include "acforbidcheater"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
