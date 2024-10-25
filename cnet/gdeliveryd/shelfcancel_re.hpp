
#ifndef __GNET_SHELFCANCEL_RE_HPP
#define __GNET_SHELFCANCEL_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class ShelfCancel_Re : public GNET::Protocol
{
	#include "shelfcancel_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
