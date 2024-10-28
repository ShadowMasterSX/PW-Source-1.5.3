
#ifndef __GNET_FACTIONLISTONLINE_HPP
#define __GNET_FACTIONLISTONLINE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factiondb.h"
#include "factionlistonline_re.hpp"

namespace GNET
{

class FactionListOnline : public GNET::Protocol
{
	#include "factionlistonline"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		FactionListOnline_Re re;
		re.localsid = localsid;
		Factiondb::GetInstance()->ListOnlineFaction(re.fid_list, re.fid_master, re.proclaim);
		manager->Send( sid,re );
	}
};

};

#endif
