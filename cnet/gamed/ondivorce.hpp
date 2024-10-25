
#ifndef __GNET_ONDIVORCE_HPP
#define __GNET_ONDIVORCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void player_on_divorce(int id1,int id2);
namespace GNET
{

class OnDivorce : public GNET::Protocol
{
	#include "ondivorce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		player_on_divorce(rid1,rid2);
	}
};

};

#endif
