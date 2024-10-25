
#ifndef __GNET_GMSHUTUP_HPP
#define __GNET_GMSHUTUP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryclient.hpp"
#include "glinkserver.hpp"
#include "gmshutup_re.hpp"
#include "privilege.hxx"
namespace GNET
{

class GMShutup : public GNET::Protocol
{
	#include "gmshutup"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (manager==lsm)
		{
			//GM禁言玩家，dstuserid实际是被封禁玩家的roleid
			if (! lsm->PrivilegeCheck(sid, gmroleid,Privilege::PRV_FORBID_TALK) )
			{
				Log::log(LOG_ERR,"WARNING: user %d try to use PRV_FORBID_TALK privilege that he doesn't have.\n",gmroleid);
				return;
			}
			char content[256];
			sprintf(content,"ShutupUser: shutupUserid=%d,forbidtime=%d",dstuserid,forbid_time);
			Log::gmoperate(gmroleid,Privilege::PRV_FORBID_TALK,content);	
			
			GDeliveryClient::GetInstance()->SendProtocol(this);
			lsm->Send(sid,GMShutup_Re(ERR_SUCCESS,dstuserid,forbid_time));
		}
		else
		{
			//来自delivery
			lsm->shutupusermap[dstuserid]=GRoleForbid(Privilege::PRV_FORBID_TALK,forbid_time,0/*create time*/,reason);
		}

	}
};

};

#endif
