
#ifndef __GNET_GMRESTARTSERVER_HPP
#define __GNET_GMRESTARTSERVER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "gproviderserver.hpp"
#include "gdeliveryclient.hpp"
#include "privilege.hxx"
namespace GNET
{

class GMRestartServer : public GNET::Protocol
{
	#include "gmrestartserver"
	
	class DispatchData
	{
		const GMRestartServer* restart;
	public:
		DispatchData(const GMRestartServer* msg):restart(msg) {}
		~DispatchData() {}
		void operator() (std::pair<const int, GNET::RoleData> pair)
		{
			if (pair.second.status == _STATUS_ONGAME)
				GLinkServer::GetInstance()->Send(pair.second.sid,restart);
		}	
	};

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		GDeliveryClient* dsm=GDeliveryClient::GetInstance();
		//来自客户端的协议，检查用户是不是GM，如果是，则转发给delivery
		if (manager==lsm)
		{
			return;	//禁止了
			//DEBUG_PRINT("gdelivery::gmrestart: receive from client(roleid %d,localsid %d,gsid %d)\n",gmroleid,localsid,gsid);
			if (! lsm->PrivilegeCheck(sid, gmroleid,Privilege::PRV_SHUTDOWN_GAMESERVER) )
			{
				Log::log(LOG_ERR,"WARNING: user %d try to use GM_OP_RESTART privilege that he doesn't have.\n",gmroleid);
				return;
			}

			char content[256];
			sprintf(content,"ShutdownGameServer, Force all user offline.");
			Log::gmoperate(gmroleid,Privilege::PRV_SHUTDOWN_GAMESERVER,content);
			
			this->localsid=sid;
			dsm->SendProtocol(this);
		}
		else if (manager==dsm) //来自Delivery的消息，则转发给所有的客户端，告知服务器重启消息
		{
			//DEBUG_PRINT("gdelivery::gmrestart: receive from delivery(roleid %d,localsid %d,gsid %d)\n",gmroleid,localsid,gsid);
			this->gmroleid=-1;
			this->localsid=0;
			this->gsid=-1;
			if (lsm->roleinfomap.size())
				std::for_each(lsm->roleinfomap.begin(),lsm->roleinfomap.end(),DispatchData(this));
		}
	}
};

};

#endif
