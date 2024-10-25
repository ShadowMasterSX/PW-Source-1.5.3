
#ifndef __GNET_REFWITHDRAWBONUS_HPP
#define __GNET_REFWITHDRAWBONUS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "refwithdrawbonus_re.hpp"


namespace GNET
{

class RefWithdrawBonus : public GNET::Protocol
{
	#include "refwithdrawbonus"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		RefWithdrawBonus_Re re;
		re.roleid = roleid;
		re.localsid = localsid;
		re.retcode = ReferenceManager::GetInstance()->WithdrawBonus(roleid);
		manager->Send(sid, re);
	}
};

};

#endif
