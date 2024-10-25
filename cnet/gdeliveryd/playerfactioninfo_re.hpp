
#ifndef __GNET_PLAYERFACTIONINFO_RE_HPP
#define __GNET_PLAYERFACTIONINFO_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "pfactioninfo"
namespace GNET
{

class PlayerFactionInfo_Re : public GNET::Protocol
{
	#include "playerfactioninfo_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (retcode==ERR_SUCCESS)
		{
			for (size_t i=0;i<faction_info.size();i++)
			{
				PFactionInfo& pfi=faction_info[i];
			}
		}
	}
};

};

#endif
