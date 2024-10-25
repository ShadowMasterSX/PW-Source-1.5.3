
#ifndef __GNET_UPDATEREMAINTIME_HPP
#define __GNET_UPDATEREMAINTIME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "mapremaintime.h"
#include "mapuser.h"
#include "gdeliveryserver.hpp"
namespace GNET
{

class UpdateRemainTime : public GNET::Protocol
{
	#include "updateremaintime"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//free_time_end=free_time_left+time(NULL);
		Log::formatlog("UpdateTime","userid=%d,remain_time=%d,free_left=%d,free_end=%d\n",userid,
				remain_time,free_time_left,free_time_end );
		RemainTime::GetInstance().UpdatePlayTime( 
				userid,
				RemainTime::play_time_t(remain_time,free_time_left,free_time_end) 
				);
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * pinfo = UserContainer::GetInstance().FindUser( userid );
		if ( NULL!=pinfo )
		{
			localsid=pinfo->localsid;
			GDeliveryServer::GetInstance()->Send( pinfo->linksid,this );
		}
	}
};

};

#endif
