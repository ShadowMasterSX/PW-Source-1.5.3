
#ifndef __GNET_CREATEFACTIONFORTRESS_HPP
#define __GNET_CREATEFACTIONFORTRESS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gfactionfortressinfo"

namespace GNET
{

class CreateFactionFortress : public GNET::Protocol
{
	#include "createfactionfortress"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		FactionBriefInfo info(factionid);
		Factiondb::GetInstance()->FindFactionInCache(info);
		if(factionid > 0)//３级帮才可以．让ｇｓ超时
			GFactionServer::GetInstance()->DispatchProtocol(0, this);
	}
};

};

#endif
