
#ifndef __GNET_ACKICKOUTUSER_HPP
#define __GNET_ACKICKOUTUSER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmkickoutuser.hpp"
#include "gdeliveryserver.hpp"
#include "gauthclient.hpp"
#include "dbforbiduser.hrp"
#include "acforbidcheater.hpp"

namespace GNET
{

class ACKickoutUser : public GNET::Protocol
{
	#include "ackickoutuser"
	
	void KickoutLocalUser(int userid)
	{
		UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);
		if(NULL != pinfo) {
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, KickoutUser(userid, pinfo->localsid, (forbid_time != -1) ? ERR_ACKICKOUT : 0));
			UserContainer::GetInstance().UserLogout(pinfo, KICKOUT_LOCAL, true);
		}
	}
	
	int SendAUForbidUser(int userid)
	{
		int ret = ERR_COMMUNICATION;
		
		if(idtype == 0) 
		{
			//add user to forbidlogin map
			if( forbid_time > 1 ) 
			{
				GRoleForbid forbid(Forbid::FBD_FORBID_LOGIN, forbid_time, time(NULL), reason); 
				ForbidLogin::GetInstance().SetForbidLoginIfLonger( userid, forbid );
			}
			
			// send to Auth,let AU send kickoutuser command
			if( forbid_time >= 0 ) 
			{ 
				if(GAuthClient::GetInstance()->GetVersion() == 1) 
				{
					//国内AU
					ACForbidCheater acforbid;
					acforbid.userid = userid;
					acforbid.time = forbid_time;
					acforbid.operation = (forbid_time > 1 ? 0 : 1);
					acforbid.reason.swap(reason);

					if(GAuthClient::GetInstance()->SendProtocol(acforbid)) 
					{
						ret = ERR_SUCCESS;
					} 
				} 
				else 
				{
					GMKickoutUser gmkou;
					gmkou.gmroleid = gmuserid;
					gmkou.kickuserid = userid;
					gmkou.forbid_time = forbid_time;
					gmkou.reason.swap(reason);

					if( GAuthClient::GetInstance()->SendProtocol(gmkou) ) 
					{
						ret = ERR_SUCCESS;
					}
				}

			}
		} 
		else if(forbid_time!=-1) 
		{
			char oper = forbid_time > 1 ? 1 : 2; // 1 封禁  2 解封
			char source = 2;
			/*gamedbd根据source判断封禁是反外挂特殊封禁(1)还是普通封禁(2)
			 *国内AU区分封禁是来自反外挂还是客服，source为1表示反外挂特殊封禁
			 *海外AU不区分封禁来源，source为2表示普通封禁*/
			if(GAuthClient::GetInstance()->GetVersion() == 1)
				source = 1;

			DBForbidUser *rpc = (DBForbidUser *)Rpc::Call(RPC_DBFORBIDUSER, ForbidUserArg(oper, gmuserid, source, userid, forbid_time, reason));

			if( GameDBClient::GetInstance()->SendProtocol( rpc ) ) 
			{
				ret = ERR_SUCCESS;
			}
		}
		
		return ret;
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(manager == CentralDeliveryClient::GetInstance()) 
		{
			//封禁信息是跨服转发过来的，由于跨服已经做过处理，此时的userid必然是账号ID
			//将本地用户踢出
			if(forbid_time != 1) KickoutLocalUser(userid);
			//通知AU相应处理，不需要向GM通知处理结果，因为GM角色此时再跨服上
			SendAUForbidUser(userid); 
		} 
		else //if(mananger == GDeliveryserver::GetInstance())
		{
			//封禁信息来自本地, 可能是原服，也可能是跨服
			bool is_central = GDeliveryServer::GetInstance()->IsCentralDS();

			STAT_MIN5("ACKickoutUser", 1);
			LOG_TRACE("ACKickoutUser: GM=%d,idtype=%d,userid=%d,forbid_time=%d,reason_size=%d",gmuserid,idtype,userid,forbid_time,reason.size());

			if( idtype != 0 ) // byroleid
			{
				int uid = UidConverter::Instance().Roleid2Uid(userid);
				if(!uid)
				{
					Roleid2Uid::LegacyFetch(manager, sid, this->Clone(), userid);
					return;
				}
				userid = uid; //此时的userid已经是真正的账号ID
			}

			//将本地账号踢出
			if(forbid_time != 1) KickoutLocalUser(userid);
			
			if(!is_central) //原服，保持正常逻辑
			{
				//通知AU相应处理，通知GM处理结果
				int ret = SendAUForbidUser(userid); 
				manager->Send(sid, GMKickoutUser_Re(ret, gmuserid, 0, userid));
			}
			else //跨服
			{
				UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);

				//将该协议转发到原服
				if(pinfo && pinfo->src_zoneid && CentralDeliveryServer::GetInstance()->DispatchProtocol(pinfo->src_zoneid, this))
				{
					manager->Send(sid, GMKickoutUser_Re(ERR_SUCCESS, gmuserid, 0, userid));
				}
				else
				{
					manager->Send(sid, GMKickoutUser_Re(ERR_COMMUNICATION, gmuserid, 0, userid));
				}
			}
		}
	}
};

};

#endif
