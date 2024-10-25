
#ifndef __GNET_PLAYERCHANGEDS_RE_HPP
#define __GNET_PLAYERCHANGEDS_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "fetchplayerdata.hrp"
#include "gamedbclient.hpp"
#include "mapforbid.h"

namespace GNET
{

class PlayerChangeDS_Re : public GNET::Protocol
{
	#include "playerchangeds_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("CrossRelated Recv PlayerChangeDS_Re retcode %d roleid %d flag %d", retcode, roleid, flag);
		
		GDeliveryServer* lsm = GDeliveryServer::GetInstance();
		PlayerInfo* pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		
		if(NULL == pinfo /*|| pinfo->localsid != localsid || BlockedRole::GetInstance()->IsRoleBlocked(roleid)*/) return;
		
		if( ERR_SUCCESS == retcode ) {
			//与PlayerLogin保持一致
			UserInfo* user = pinfo->user;

			FetchPlayerData* rpc = (FetchPlayerData*)Rpc::Call(RPC_FETCHPLAYERDATA, FetchPlayerDataArg(roleid, user->userid, flag));
			GameDBClient::GetInstance()->SendProtocol(rpc);
			
			//跨服前，先将角色logout
			UserContainer::GetInstance().RoleLogout(user, true/*表示是跨服相关logout*/);
			
			//只可能是GS发过来的
			if (flag == DS_TO_CENTRALDS) {
				user->status = _STATUS_REMOTE_HALFLOGIN;
			} else if (flag == CENTRALDS_TO_DS) {
				user->status = _STATUS_REMOTE_CACHERANDOM;
			} else {
				Log::log(LOG_ERR, "CrossRelated Recv PlayerChangeDS_Re invalid flag %d roleid %d userid %d", flag, roleid, user->userid);
			}
			
			RemoteLoggingUsers::GetInstance().Push(user->userid, roleid, user->status);
			
			Octets random_key;
			Security* rand = Security::Create(RANDOM);
			rand->Update(random_key.resize(32));
			rand->Destroy();
			user->rand_key = random_key;
		} else {
			LOG_TRACE("CrossRelated PlayerChangeDS errno %d roleid %d", retcode, roleid);
			lsm->Send(pinfo->linksid, DisconnectPlayer( roleid, -1, pinfo->localsid, -1 ));
			UserContainer::GetInstance().UserLogout(pinfo->user);
		}
	}
};

};

#endif
