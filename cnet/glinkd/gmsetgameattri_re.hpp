
#ifndef __GNET_GMSETGAMEATTRI_RE_HPP
#define __GNET_GMSETGAMEATTRI_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class GMSetGameAttri_Re : public GNET::Protocol
{
	#include "gmsetgameattri_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
