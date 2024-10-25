
#ifndef __GNET_GMKICKOUTUSER_HPP
#define __GNET_GMKICKOUTUSER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "gmkickoutuser_re.hpp"
#include "kickoutuser.hpp"
#include "gauthclient.hpp"
#include "forbid.hxx"
#include "mapforbid.h"
#include "dbforbiduser.hrp"
#include "gamemaster.h"
namespace GNET
{

class GMKickoutUser : public GNET::Protocol
{
	#include "gmkickoutuser"
	
	void KickoutLocalUser(int userid)
	{
		UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);
		if(NULL != pinfo) 
		{
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, KickoutUser(userid, pinfo->localsid, ERR_KICKOUT));
			UserContainer::GetInstance().UserLogout(pinfo, KICKOUT_LOCAL, true);
		}
	}
	
	void SendDBForbidUser(int masterid, int userid)
	{
		DBForbidUser* rpc = (DBForbidUser*)Rpc::Call(RPC_DBFORBIDUSER, ForbidUserArg(1, masterid, 2, userid, forbid_time, reason));
		GameDBClient::GetInstance()->SendProtocol(rpc);
	}
	
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(manager == CentralDeliveryClient::GetInstance()) 
		{
			//封禁信息是跨服转发过来的
			int userid = kickuserid; //跨服发来的kickoutuserid就是真正的账号ID
			int masterid = 888;
			
			Log::formatlog("gamemaster","GMKickoutUser:GM=%d:userid=%d:forbid_time=%d:reason_size=%d", masterid, userid, forbid_time, reason.size());

			//add user to forbidlogin map
			if( forbid_time >= 1 )  KickoutLocalUser(userid);
			
			if( forbid_time > 0 ) 
			{
				//不需向GM返回结果，因为GM角色在跨服，这里是原服
				SendDBForbidUser(masterid, userid);
			}
		} 
		else //if(mananger == GDeliveryserver::GetInstance())
		{
			//封禁信息来自本地, 可能是原服，也可能是跨服
			bool is_central = GDeliveryServer::GetInstance()->IsCentralDS();

			int userid = UidConverter::Instance().Roleid2Uid(kickuserid);
			if(!userid)
			{
				Roleid2Uid::LegacyFetch(manager, sid, this->Clone(), kickuserid);
				return;
			}

			GameMaster* master = MasterContainer::Instance().Find(gmroleid);
			if(master == NULL) return;
			int masterid = master->userid;
			
			Log::formatlog("gamemaster","GMKickoutUser:GM=%d:userid=%d:forbid_time=%d:reason_size=%d", masterid, userid, forbid_time, reason.size());

			//add user to forbidlogin map
			if( forbid_time >= 1 ) KickoutLocalUser(userid);

			if( forbid_time > 0 ) 
			{
				if(!is_central) //原服
				{
					//向DB要求封禁User
					SendDBForbidUser(masterid, userid);
					//向GM返回结果
					manager->Send(sid, GMKickoutUser_Re(ERR_SUCCESS, gmroleid, localsid, userid));
				} 
				else //跨服
				{
					UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);
					if(pinfo == NULL) return;

					CrossInfoData* pCrsRole = pinfo->GetCrossInfo(kickuserid);
					if(pCrsRole && pCrsRole->src_zoneid != 0) //可以找到要封禁User来自的zoneid
					{	
						//转发封禁到原服
						this->kickuserid = userid; //转发前将kickoutuserid置为正确的账号ID
						if(CentralDeliveryServer::GetInstance()->DispatchProtocol(pCrsRole->src_zoneid, this)) 
						{
							LOG_TRACE("GMKickoutUser:GM=%d:userid=%d:Dispatch to src_zoneid %d", masterid, userid, pCrsRole->src_zoneid);
							//向GM返回结果
							manager->Send(sid, GMKickoutUser_Re(ERR_SUCCESS, gmroleid, localsid, userid));
						} 
					} 
				}
			}
		}


		/*int userid = UidConverter::Instance().Roleid2Uid(kickuserid);
		if(!userid)
		{
			Roleid2Uid::LegacyFetch(manager, sid, this->Clone(), kickuserid);
			return;
		}
		GameMaster* master = MasterContainer::Instance().Find(gmroleid);
		if(!master)
			return;
		Log::formatlog("gamemaster","GMKickoutUser:GM=%d:userid=%d:forbid_time=%d:reason_size=%d", master->userid, userid, forbid_time, reason.size());

		//add user to forbidlogin map
		if ( forbid_time>=1 )
		{
			UserInfo * pinfo = UserContainer::GetInstance().FindUser(userid);
			if (NULL!=pinfo)
			{
				GDeliveryServer::GetInstance()->Send(pinfo->linksid,KickoutUser(userid, pinfo->localsid, ERR_KICKOUT));
				UserContainer::GetInstance().UserLogout(pinfo,true);
			}
		}
		if ( forbid_time > 0 )
		{
			DBForbidUser *rpc = (DBForbidUser *)Rpc::Call(RPC_DBFORBIDUSER, ForbidUserArg(1, master->userid, 2, userid, forbid_time, reason));
			GameDBClient::GetInstance()->SendProtocol(rpc);
			manager->Send(sid,GMKickoutUser_Re(ERR_SUCCESS,gmroleid,localsid,userid));
		}*/
	}
};

};

#endif
