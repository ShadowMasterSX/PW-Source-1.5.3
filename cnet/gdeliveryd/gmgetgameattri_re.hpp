
#ifndef __GNET_GMGETGAMEATTRI_RE_HPP
#define __GNET_GMGETGAMEATTRI_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class GMGetGameAttri_Re : public GNET::Protocol
{
	#include "gmgetgameattri_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
