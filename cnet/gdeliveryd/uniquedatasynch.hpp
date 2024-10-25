
#ifndef __GNET_UNIQUEDATASYNCH_HPP
#define __GNET_UNIQUEDATASYNCH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class UniqueDataSynch : public GNET::Protocol
{
	#include "uniquedatasynch"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
