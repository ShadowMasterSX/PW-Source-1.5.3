
#ifndef __GNET_GMSHUTUP_RE_HPP
#define __GNET_GMSHUTUP_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmchoice.h"
#include "glinkclient.hpp"
namespace GNET
{

class GMShutup_Re : public GNET::Protocol
{
	#include "gmshutup_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		printf("Shutup user %d, forbid time is %d\n",dstuserid,forbid_time);
		GMChoice(GLinkClient::GetInstance()->roleid,manager,sid);
	}
};

};

#endif
