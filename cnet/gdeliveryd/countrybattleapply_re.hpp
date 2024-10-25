
#ifndef __GNET_COUNTRYBATTLEAPPLY_RE_HPP
#define __GNET_COUNTRYBATTLEAPPLY_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "countrybattleapplyentry"

namespace GNET
{

class CountryBattleApply_Re : public GNET::Protocol
{
	#include "countrybattleapply_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
