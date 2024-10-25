
#ifndef __GNET_GMKICKOUTROLE_RE_HPP
#define __GNET_GMKICKOUTROLE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmchoice.h"
#include "glinkclient.hpp"
namespace GNET
{

class GMKickoutRole_Re : public GNET::Protocol
{
	#include "gmkickoutrole_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (retcode==ERR_SUCCESS)
			printf("kickoutrole %d successfully.\n",kickroleid);
		else
			printf("kickoutrole %d failed.\n",kickroleid);
		GMChoice(GLinkClient::GetInstance()->roleid,manager,sid);

	}
};

};

#endif
