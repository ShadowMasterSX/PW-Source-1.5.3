
#ifndef __GNET_ADDCASH_HPP
#define __GNET_ADDCASH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "addcash_re.hpp"
#include "mapuser.h"
#include "gauthclient.hpp"

namespace GNET
{

class AddCash : public GNET::Protocol
{
	#include "addcash"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(cash < 0)
		{
			Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
			UserInfo * userinfo = UserContainer::GetInstance().FindUser(userid);
			if(userinfo && userinfo->status == _STATUS_ONGAME)
			{
				GAuthClient::GetInstance()->SendProtocol(AddCash_Re(CASH_ADD_FAILED, userid, zoneid, sn));	
				return;
			}		
		}
		GameDBClient::GetInstance()->SendProtocol( *this );
	}
};

};

#endif
