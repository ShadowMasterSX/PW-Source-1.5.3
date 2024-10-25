
#ifndef __GNET_KEGETSTATUS_HPP
#define __GNET_KEGETSTATUS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "kingelection.h"
#include "kegetstatus_re.hpp"

namespace GNET
{

class KEGetStatus : public GNET::Protocol
{
	#include "kegetstatus"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("kegetstatus: receive. roleid=%d\n", roleid);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL!=pinfo )
		{
			KEGetStatus_Re re;
			re.localsid = pinfo->localsid;
			KingElection::GetInstance().GetStatus(re.status, re.king, re.candidate_list);
			GDeliveryServer::GetInstance()->Send(pinfo->linksid,re);
		}
	}
};

};

#endif
