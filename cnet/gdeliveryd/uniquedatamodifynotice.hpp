
#ifndef __GNET_UNIQUEDATAMODIFYNOTICE_HPP
#define __GNET_UNIQUEDATAMODIFYNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class UniqueDataModifyNotice : public GNET::Protocol
{
	#include "uniquedatamodifynotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
