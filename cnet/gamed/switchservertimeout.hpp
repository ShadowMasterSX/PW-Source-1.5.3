
#ifndef __GNET_SWITCHSERVERTIMEOUT_HPP
#define __GNET_SWITCHSERVERTIMEOUT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void switch_server_timeout(int linkd, int userid, int local_sid);
namespace GNET
{

class SwitchServerTimeout : public GNET::Protocol
{
	#include "switchservertimeout"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		switch_server_timeout(link_id,roleid,localsid);
	}
};

};

#endif
