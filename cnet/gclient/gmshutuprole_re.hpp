
#ifndef __GNET_GMSHUTUPROLE_RE_HPP
#define __GNET_GMSHUTUPROLE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmchoice.h"
#include "glinkclient.hpp"
namespace GNET
{

class GMShutupRole_Re : public GNET::Protocol
{
	#include "gmshutuprole_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		printf("Shutup role %d, forbid time is %d(seconds)\n",dstroleid,forbid_time);
		GMChoice(GLinkClient::GetInstance()->roleid,manager,sid);
	}
};

};

#endif
