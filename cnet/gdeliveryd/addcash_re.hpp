
#ifndef __GNET_ADDCASH_RE_HPP
#define __GNET_ADDCASH_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "addcashnotify.hpp"
#include "mapuser.h"
#include "gproviderserver.hpp"

namespace GNET
{

class AddCash_Re : public GNET::Protocol
{
	#include "addcash_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GAuthClient::GetInstance()->SendProtocol( *this );
		if(retcode == ERR_SUCCESS)
		{
			Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());	
			UserInfo * userinfo = UserContainer::GetInstance().FindUser(userid);
			if(userinfo && userinfo->status == _STATUS_ONGAME)
			{
				GProviderServer::GetInstance()->DispatchProtocol(userinfo->gameid, AddCashNotify(userinfo->roleid));	
			}
		}
	}
};

};

#endif
