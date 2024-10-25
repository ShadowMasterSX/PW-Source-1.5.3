
#ifndef __GNET_ADDCASH_RE_HPP
#define __GNET_ADDCASH_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AddCash_Re : public GNET::Protocol
{
	#include "addcash_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
