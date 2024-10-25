
#ifndef __GNET_GMONLINENUM_RE_HPP
#define __GNET_GMONLINENUM_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gmchoice.h"
namespace GNET
{

class GMOnlineNum_Re : public GNET::Protocol
{
	#include "gmonlinenum_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		printf("Total onlineuser is %d (local link is %d)\n",total_num,local_num);
		GMChoice(gmroleid,manager,sid);
	}
};

};

#endif
