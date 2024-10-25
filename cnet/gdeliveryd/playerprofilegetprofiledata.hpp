
#ifndef __GNET_PLAYERPROFILEGETPROFILEDATA_HPP
#define __GNET_PLAYERPROFILEGETPROFILEDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "playerprofileman.h"

namespace GNET
{

class PlayerProfileGetProfileData : public GNET::Protocol
{
	#include "playerprofilegetprofiledata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		PlayerProfileMan::GetInstance()->GetPlayerProfileData(roleid, sid, localsid);
	}
};

};

#endif
