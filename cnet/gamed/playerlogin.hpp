
#ifndef __GNET_PLAYERLOGIN_HPP
#define __GNET_PLAYERLOGIN_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#ifdef _TESTCODE
#include "playerlogin_re.hpp"
#include "playerlogout.hpp"
#endif

void user_login(int cs_index,int sid,int uid,const void * auth_buf, size_t auth_size, bool isshielduser, char flag);
namespace GNET
{

class PlayerLogin : public GNET::Protocol
{
	#include "playerlogin"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
#ifdef _TESTCODE //liping
#ifdef _WAIT_TIME
		sleep(5);
#endif
		DEBUG_PRINT ("gamed(%d):: receive playerlogin, roleid=%d,link_id=%d,localsid=%d\n",GProviderClient::GetGameServerID(),roleid,provider_link_id,localsid);
		manager->Send(sid,PlayerLogin_Re(ERR_SUCCESS,roleid,provider_link_id,localsid));
#endif
		user_login(provider_link_id,localsid,roleid, (const void*) &(*auth.begin()),auth.size(),usbbind,flag);
	}
};

};

#endif
