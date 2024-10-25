
#ifndef __GNET_FACTIONLEAVE_RE_HPP
#define __GNET_FACTIONLEAVE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionchoice.h"
#include "glinkclient.hpp"
namespace GNET
{

class FactionLeave_Re : public GNET::Protocol
{
	#include "factionleave_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (retcode == ERR_SUCCESS)
			printf("成功离开帮派\n");
		else
			printf("离开帮派失败\n");

		FactionChoice(roleid,manager,sid);
	}
};

};

#endif
