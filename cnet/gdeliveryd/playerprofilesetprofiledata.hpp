
#ifndef __GNET_PLAYERPROFILESETPROFILEDATA_HPP
#define __GNET_PLAYERPROFILESETPROFILEDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "playerprofiledata"
#include "playerprofileman.h"

namespace GNET
{

class PlayerProfileSetProfileData : public GNET::Protocol
{
	#include "playerprofilesetprofiledata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		PlayerProfileMan::GetInstance()->SetPlayerProfileData(roleid, data);
	}
};

};

#endif
