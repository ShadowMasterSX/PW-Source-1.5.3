
#ifndef __GNET_ADDCASH_HPP
#define __GNET_ADDCASH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AddCash : public GNET::Protocol
{
	#include "addcash"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
