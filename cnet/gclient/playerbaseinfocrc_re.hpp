
#ifndef __GNET_PLAYERBASEINFOCRC_RE_HPP
#define __GNET_PLAYERBASEINFOCRC_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class PlayerBaseInfoCRC_Re : public GNET::Protocol
{
	#include "playerbaseinfocrc_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (result==ERR_SUCCESS)
		{
			for (size_t i=0;i<playerlist.size();i++)
			{
				DEBUG_PRINT("player %d 's CRC is %d.\n",playerlist[i],CRClist[i]);
			}
		}
		else
		{
			DEBUG_PRINT("Get rolebaseinfo CRC failed.\n");
		}
	}
};

};

#endif
