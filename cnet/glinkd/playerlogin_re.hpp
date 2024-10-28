
#ifndef __GNET_PLAYERLOGIN_RE_HPP
#define __GNET_PLAYERLOGIN_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "selectrole_re.hpp"
#include "statusannounce.hpp"
#include "glinkserver.hpp"
namespace GNET
{

class PlayerLogin_Re : public GNET::Protocol
{
	#include "playerlogin_re"
	void ConvertErrCode()
	{
		if (result==-1) result=ERR_GAMEDB_FAIL;
		else if (result==1) result=ERR_ENTERWORLD_FAIL;
		else if (result==2) result=ERR_EXCEED_MAXNUM;
		else if (result==3 || result==4) result=ERR_IN_WORLD;
		else if (result==5) result=ERR_INSTANCE_OVERFLOW;
		else if (result==6) result=ERR_INVALID_LINEID;
		else if (result==ERR_SERVEROVERLOAD) result=ERR_SERVEROVERLOAD;
		else result=ERR_ENTERWORLD_FAIL;
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		//change state of linkserver
		
		SelectRole_Re cmd(result);
		if (result!=ERR_SUCCESS)
		{
			DEBUG_PRINT("glinkd::playerlogin_re from delivery. login failed. result=%d,roleid=%d,gameid=%d,localsid=%d\n",
					result,roleid,src_provider_id,localsid);
			if (result == ERR_ROLEFORBID|| result == ERR_PRP_RESETPOS_OK) // 禁用号 或则 位置重置成功 都重置 选人状态
			{
				if(flag == 0)
				{
					lsm->ChangeState(localsid,&state_GSelectRoleServer);
				}
				else
				{
					Log::log(LOG_DEBUG,"glinkd::cross server(flag=%d)login failed,result=%d,roleid=%d,gameid=%d,localsid=%d",
							flag,result,roleid,src_provider_id,localsid);
					lsm->ChangeState(localsid, &state_Null);
				}
			}
			else
			{
				ConvertErrCode();
				cmd.result = result;
				lsm->ChangeState(localsid,&state_Null);
			}
		}
		else
		{
			lsm->RoleLogin(localsid, roleid, src_provider_id, cmd.auth);
		}
		GLinkServer::GetInstance()->Send(localsid,cmd);
	}
};

};

#endif
