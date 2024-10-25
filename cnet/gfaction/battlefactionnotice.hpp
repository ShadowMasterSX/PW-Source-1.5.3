
#ifndef __GNET_BATTLEFACTIONNOTICE_HPP
#define __GNET_BATTLEFACTIONNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "factiondb.h"


namespace GNET
{

class BattleFactionNotice : public GNET::Protocol
{
	#include "battlefactionnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		for(std::set<int>::iterator it=factionids.begin(); it!=factionids.end(); ++it)
			Factiondb::GetInstance()->ObtainFactionInfo(*it);
	}
};

};

#endif
