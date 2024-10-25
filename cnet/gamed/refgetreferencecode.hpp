
#ifndef __GNET_REFGETREFERENCECODE_HPP
#define __GNET_REFGETREFERENCECODE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

bool query_player_level(int role, int & level, int & reputation);
namespace GNET
{

class RefGetReferenceCode : public GNET::Protocol
{
	#include "refgetreferencecode"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		RefGetReferenceCode_Re re;
		re.retcode = 0;
		re.roleid = roleid;
		query_player_level(roleid, re.level, re.reputation);
		re.localsid = localsid;
		manager->Send(sid,re);
	}
};

};

#endif
