
#ifndef __GNET_ACQPROTOCOLSTATS_HPP
#define __GNET_ACQPROTOCOLSTATS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "acprotocolstat"
#include "userdata.hpp"

namespace GNET
{

class ACQProtocolStats : public GNET::Protocol
{
	#include "acqprotocolstats"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}

	void operator() (const UserSessionData *data)
	{
		const std::deque<ProtoStat>& ps = data->GetProtoStats();
		for(std::deque<ProtoStat>::const_iterator it = ps.begin(), ie = ps.end(); it != ie; ++it)
		{
			const ProtoStat &p = (*it);
			proto_stats.push_back(ACProtocolStat(p.keepalive, p.gamedatasend, p.acreport));
		}
		proto_stats.push_back(ACProtocolStat(0, 0, data->GetLastProtoStatTime()));
	}
};

};

#endif
