
#ifndef __GNET_GMSHUTUPROLE_HPP
#define __GNET_GMSHUTUPROLE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryclient.hpp"
#include "glinkserver.hpp"
#include "gmshutuprole_re.hpp"
#include "privilege.hxx"
namespace GNET
{

class GMShutupRole : public GNET::Protocol
{
	#include "gmshutuprole"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (manager==lsm)
		{
			//只有协议来自客户端才做下面的操作，来自delivery不做此操作
			if (! lsm->PrivilegeCheck(sid, gmroleid,Privilege::PRV_FORBID_TALK) )
			{
				Log::log(LOG_ERR,"WARNING: user %d try to use PRV_FORBID_TALK privilege that he doesn't have.\n",gmroleid);
				return;
			}

			char content[256];
			sprintf(content,"ShutupRole: shutupRoleid=%d,forbidtime=%d",dstroleid,forbid_time);
			Log::gmoperate(gmroleid,Privilege::PRV_FORBID_TALK,content);			

			GDeliveryClient::GetInstance()->SendProtocol(this);
			lsm->Send(sid,GMShutupRole_Re(ERR_SUCCESS,dstroleid,forbid_time));
		}
		{
			lsm->shutuprolemap[dstroleid]=GRoleForbid(Privilege::PRV_FORBID_TALK,forbid_time,0/*create time*/,reason);
		}

	}
};

};

#endif
