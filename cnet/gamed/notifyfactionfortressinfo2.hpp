
#ifndef __GNET_NOTIFYFACTIONFORTRESSINFO2_HPP
#define __GNET_NOTIFYFACTIONFORTRESSINFO2_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gfactionfortressinfo2"

bool notify_faction_fortress_data(GNET::faction_fortress_data2 * data2);

namespace GNET
{

void DB2FactionFortressData(int factionid, const GFactionFortressInfo2 * pData, faction_fortress_data2 & data2);

class NotifyFactionFortressInfo2 : public GNET::Protocol
{
	#include "notifyfactionfortressinfo2"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		faction_fortress_data2 data2;
		DB2FactionFortressData(factionid,&info2,data2);
		notify_faction_fortress_data(&data2);
	}
};

};

#endif
