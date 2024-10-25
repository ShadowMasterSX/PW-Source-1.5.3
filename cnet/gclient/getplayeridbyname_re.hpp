
#ifndef __GNET_GETPLAYERIDBYNAME_RE_HPP
#define __GNET_GETPLAYERIDBYNAME_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkclient.hpp"
#include "gmchoice.h"
namespace GNET
{

class GetPlayerIDByName_Re : public GNET::Protocol
{
	#include "getplayeridbyname_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (retcode==ERR_SUCCESS)
			printf("Player's roleid is %d(userid %d)\n",roleid,roleid-roleid % 16);
		else
			printf("Can not find player's id\n");
		GMChoice(GLinkClient::GetInstance()->roleid,manager,sid);

	}
};

};

#endif
