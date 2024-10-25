
#ifndef __GNET_REFLISTREFERRALS_HPP
#define __GNET_REFLISTREFERRALS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "referencemanager.h"
#include "reflistreferrals_re.hpp"


namespace GNET
{

class RefListReferrals : public GNET::Protocol
{
	#include "reflistreferrals"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		RefListReferrals_Re list_re;
		list_re.roleid = roleid;
		list_re.start_index = start_index;
		list_re.localsid = localsid;
		list_re.retcode = ReferenceManager::GetInstance()->ListReferrals(roleid, start_index, list_re.total, list_re.bonus_avail_today, list_re.referrals);
		manager->Send(sid, list_re);
	}
};

};

#endif
