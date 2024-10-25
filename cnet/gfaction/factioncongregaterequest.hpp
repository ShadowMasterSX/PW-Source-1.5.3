
#ifndef __GNET_FACTIONCONGREGATEREQUEST_HPP
#define __GNET_FACTIONCONGREGATEREQUEST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gfactionserver.hpp"
#include "gproviderserver.hpp"

namespace GNET
{

class FactionCongregateRequest : public GNET::Protocol
{
	#include "factioncongregaterequest"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		typedef std::map<int/*gsid*/, std::vector<int/*roleid*/> > gs_role_map;
		
		gs_role_map role_map;
		{
			GFactionServer* gfs=GFactionServer::GetInstance();
			Thread::Mutex::Scoped l(gfs->locker_player);
			for(GFactionServer::FactionMemberMap::iterator
					itb=gfs->factionmembermap.lower_bound(factionid),
					ite=gfs->factionmembermap.upper_bound(factionid);
					itb!=ite; ++itb)
			{
				if ((*itb).second && (*itb).second->roleid != sponsor)
					role_map[(*itb).second->gsid].push_back((*itb).second->roleid);
			}
		}
		for(gs_role_map::iterator it=role_map.begin(); it!=role_map.end(); ++it)
		{
			this->member.swap(it->second);
			GProviderServer::GetInstance()->DispatchProtocol(it->first,this);
		}
	}
};

};

#endif
