
#ifndef __GNET_GMKICKOUTUSER_RE_HPP
#define __GNET_GMKICKOUTUSER_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gmchoice.h"
#include "glinkclient.hpp"
namespace GNET
{

class GMKickoutUser_Re : public GNET::Protocol
{
	#include "gmkickoutuser_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (retcode==ERR_SUCCESS)
			printf("kickoutuser %d successfully.\n",kickuserid);
		else
			printf("kickoutuser %d failed.\n",kickuserid);
		GMChoice(GLinkClient::GetInstance()->roleid,manager,sid);
	}
};

};

#endif
