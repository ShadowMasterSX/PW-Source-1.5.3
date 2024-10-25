
#ifndef __GNET_SENDAUMAIL_HPP
#define __GNET_SENDAUMAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class SendAUMail : public GNET::Protocol
{
	#include "sendaumail"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if (NULL == pinfo || pinfo->roleid!=roleid)
			return;
		if(pinfo->friend_ver < 0)
			return;
		FriendextinfoManager::GetInstance()->SendAUMail(pinfo,friend_id,mail_template_id);	
	}
};

};

#endif
