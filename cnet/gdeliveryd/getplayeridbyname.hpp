
#ifndef __GNET_GETPLAYERIDBYNAME_HPP
#define __GNET_GETPLAYERIDBYNAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "getplayeridbyname_re.hpp"
#include "mapuser.h"
#include "gamedbclient.hpp"
#include "getroleid.hrp"
namespace GNET
{

class GetPlayerIDByName : public GNET::Protocol
{
	#include "getplayeridbyname"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		int roleid = -1;
		if( UserContainer::GetInstance().FindRoleId(rolename, roleid) )
		{
			manager->Send(sid,GetPlayerIDByName_Re(ERR_SUCCESS,localsid,rolename,roleid,reason));
		}
		else
		{
			GetRoleId* rpc=(GetRoleId*)Rpc::Call( RPC_GETROLEID,GetRoleIdArg(rolename, reason) );
			rpc->save_sid=sid;
			rpc->save_localsid=localsid;
			GameDBClient::GetInstance()->SendProtocol( rpc );
		}
	}
};

};

#endif
