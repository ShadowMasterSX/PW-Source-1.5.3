
#ifndef __GNET_UNIQUEDATAMODIFYREQUIRE_HPP
#define __GNET_UNIQUEDATAMODIFYREQUIRE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class UniqueDataModifyRequire : public GNET::Protocol
{
	#include "uniquedatamodifyrequire"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
