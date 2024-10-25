
#ifndef __GNET_SYNCFORCEGLOBALDATA_HPP
#define __GNET_SYNCFORCEGLOBALDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gforceglobaldatabrief"

namespace GNET
{

class SyncForceGlobalData : public GNET::Protocol
{
	#include "syncforceglobaldata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
