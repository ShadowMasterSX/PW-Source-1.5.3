
#ifndef __GNET_CREATEROLE_HPP
#define __GNET_CREATEROLE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "createrole_re.hpp"
#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "uniquenameclient.hpp"

#include "dbcreaterole.hrp"
#include "precreaterole.hrp"
#include "matcher.h"
#include "mapuser.h"
#include "refspreadcode.h"
namespace GNET
{
	class CreateRole : public GNET::Protocol
	{
#include "createrole"

		int DecodeReferid(Octets &refer_id, int & referrer_district, int & referrer_roleid)
		{
			int userid;
			if (RefSpreadCode::Decode(refer_id, userid, referrer_district, referrer_roleid))
			{
				if (referrer_district == GDeliveryServer::GetInstance()->district_id)
					return ERR_SUCCESS;
				else
					return REF_ERR_REFERRERNOTINDISTRICT;
			}
			else
			{
				return REF_ERR_INVALIDSPREADCODE;
			}
		}

		void Send2UniqueNameSrv(UserInfo *user, GDeliveryServer* dsm,Manager::Session::ID sid)
		{
			if( !UniqueNameClient::GetInstance()->IsConnect() )
			{
				Log::log(LOG_ERR,"gdelivery::createrole: lose connection to uniquenamed, userid=%d.", userid);
				dsm->Send(sid,CreateRole_Re(ERR_COMMUNICATION,_ROLE_INVALID,localsid));
				return;
			}
			//不允许客户端发来上线id
			referid.clear();
   			int player_suggest_referrer = 0;
			int player_suggest_referrer_distric = 0;
			if (referid.size()>0 && user->real_referrer==0 && user->rolelist.GetRoleCount()==0)
			{
				//AU没有发来上线id 使用用户传来的上线id
				Octets temp;
				temp.swap(referid);
				CharsetConverter::conv_charset_u2t(temp, referid);
				int res = DecodeReferid(referid, player_suggest_referrer_distric, player_suggest_referrer);
				if (res != ERR_SUCCESS)
				{
					dsm->Send(sid,CreateRole_Re(res,_ROLE_INVALID,localsid));
					return;
				}
/*
				SetReferrerAUArg arg(userid, player_suggest_referrer_distric, player_suggest_referrer);
				SetReferrerAU * rpc = (SetReferrerAU*) Rpc::Call(RPC_SETREFERRERAU, arg);
				rpc->save_linksid = sid;
				rpc->save_localsid = localsid;
				rpc->roleinfo = roleinfo;
				if (!GAuthClient::GetInstance()->SendProtocol(rpc))
				{
					Log::log(LOG_ERR,"gdelivery::createrole: lose connection to au server, userid=%d.", userid);
					dsm->Send(sid,CreateRole_Re(ERR_COMMUNICATION,_ROLE_INVALID,localsid));
					return;
				}
*/
			}
			PreCreateRole* rpc=(PreCreateRole*) Rpc::Call( RPC_PRECREATEROLE,
					PreCreateRoleArg(dsm->zoneid,userid,0,roleinfo.name));
			rpc->save_linksid=sid;
			rpc->save_localsid=localsid;
			rpc->roleinfo=roleinfo;
			rpc->player_suggest_referrer = player_suggest_referrer;  
			if (!UniqueNameClient::GetInstance()->SendProtocol(rpc))
			{
				Log::log(LOG_ERR,"gdelivery::createrole: send to uniquenamed failed, userid=%d.", userid);
				dsm->Send(sid,CreateRole_Re(ERR_CREATEROLE,_ROLE_INVALID,localsid));
			}
		}
		bool ValidRolename(const Octets& rolename)
		{
			return rolename.size()>=2 && rolename.size()<=GDeliveryServer::GetInstance()->max_name_len
				&& Matcher::GetInstance()->Match((char*)rolename.begin(),rolename.size())==0;
		}
		void Process(Manager *manager, Manager::Session::ID sid)
		{
			GDeliveryServer* dsm=GDeliveryServer::GetInstance();
			if(dsm->IsCentralDS()) {       
				Log::log(LOG_ERR, "Userid %d try to create role on Central Delivery Server, refuse him!", userid);
				return; 
			}   
			
			if(roleinfo.occupation >= USER_CLASS_COUNT || roleinfo.gender > 1)
				return;
			UserInfo* userinfo = UserContainer::GetInstance().FindUser(userid);
			if (NULL==userinfo)
				return;

			/* valid name */
			if (!ValidRolename(roleinfo.name))
			{
				dsm->Send(sid,CreateRole_Re(ERR_INVALIDCHAR,_ROLE_INVALID,localsid));
				return;
			}
			/* (unique roleid version)send RoleID request to Authd */
			Send2UniqueNameSrv(userinfo, dsm,sid);	
			/* (multi roleid version)send DBCreateRole Rpc to gamedbd */
			return;
		}
	};

};

#endif
