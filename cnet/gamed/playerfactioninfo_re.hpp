
#ifndef __GNET_PLAYERFACTIONINFO_RE_HPP
#define __GNET_PLAYERFACTIONINFO_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "pfactioninfo"
namespace GNET
{
void ReceivePlayerFactionInfo(int roleid,unsigned int faction_id,char faction_role,unsigned char faction_pvp_mask,int64_t unifid);
class PlayerFactionInfo_Re : public GNET::Protocol
{
	#include "playerfactioninfo_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
#ifdef _TESTCODE
		printf("\ngamed::playerfactioninfo_re: retcode=%d\n",retcode);
		if (retcode==ERR_SUCCESS)
		{
			for (size_t i=0;i<faction_info.size();i++)
			{
				printf("player %3d's faction id is %3d, faction roleid is %d\n",faction_info[i].roleid,faction_info[i].faction_id);
			}
		}
#else
		for (size_t i=0;i<faction_info.size();i++)
			ReceivePlayerFactionInfo(faction_info[i].roleid,faction_info[i].faction_id,faction_info[i].faction_role,faction_info[i].faction_pvp_mask,faction_info[i].unifid);		
#endif
		
	}
};

};

#endif
