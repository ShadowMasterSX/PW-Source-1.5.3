
#ifndef __GNET_ADDICTIONCONTROL_HPP
#define __GNET_ADDICTIONCONTROL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gpair"
#include "mapuser.h"

namespace GNET
{

class AddictionControl : public GNET::Protocol
{
	#include "addictioncontrol"

	void Process(Manager *manager, Manager::Session::ID sid)
	{	
		LOG_TRACE( "AddictionControl: zoneid=%d,userid=%d,rate=%d,msg=%d", zoneid,userid,rate,msg);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * pinfo = UserContainer::GetInstance().FindUser(userid);
		if(NULL == pinfo)
			return;
		pinfo->acstate = (AddictionControl*)this->Clone();
		pinfo->actime = Timer::GetTime();
		if(pinfo->status==_STATUS_ONGAME)
		{
			userid = pinfo->roleid;
			GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid,this);
		}

	}
};

};

#endif
