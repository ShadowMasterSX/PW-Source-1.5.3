
#ifndef __GNET_SHELF_RE_HPP
#define __GNET_SHELF_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class Shelf_Re : public GNET::Protocol
{
	#include "shelf_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
