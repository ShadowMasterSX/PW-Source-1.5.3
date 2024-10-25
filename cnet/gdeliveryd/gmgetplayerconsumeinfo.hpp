
#ifndef __GNET_GMGETPLAYERCONSUMEINFO_HPP
#define __GNET_GMGETPLAYERCONSUMEINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "dbgetconsumeinfos.hrp"

namespace GNET
{

class GMGetPlayerConsumeInfo : public GNET::Protocol
{
	#include "gmgetplayerconsumeinfo"
	bool QueryDB( PlayerInfo& ui )
	{
		DBGetConsumeInfos* rpc=(DBGetConsumeInfos*)Rpc::Call( RPC_DBGETCONSUMEINFOS,DBGetConsumeInfosArg(playerlist) );
		rpc->save_gmroleid=ui.roleid;
		rpc->save_linksid=ui.linksid;
		rpc->save_localsid=ui.localsid;
		return GameDBClient::GetInstance()->SendProtocol( rpc );
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("gmgetplayerconsumeinfo: receive. roleid=%d,localsid=%d,num=%d\n",gmroleid,localsid,playerlist.size());
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( (gmroleid) );
		if ( NULL!=pinfo )
		{
			QueryDB( *pinfo );
		}
	}
};

};

#endif
