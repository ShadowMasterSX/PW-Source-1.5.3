
#ifndef __GNET_KICKOUTUSER2_HPP
#define __GNET_KICKOUTUSER2_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class KickoutUser2 : public GNET::Protocol
{
	#include "kickoutuser2"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		DEBUG_PRINT("glinkd::receive kickoutuser2 from delivery.userid=%d,localsid=%d,cause=%d,[manager %p]\n",userid,localsid,cause,manager);
		
		if (lsm->ValidUser(localsid,userid))
		{	
			switch (cause)
			{
			case ERR_REMOTE_DATA_INVALID:
				lsm->SessionError(localsid, cause, "invalid remote data");
				break;
			case ERR_NOT_RECALL_USER:
				lsm->SessionError(localsid, cause, "not recall user");
				break;
			default:
				lsm->SessionError(localsid, cause, "");
				break;
			}
		}
	}
};

};

#endif
