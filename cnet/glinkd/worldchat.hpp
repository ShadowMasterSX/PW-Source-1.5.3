
#ifndef __GNET_WORLDCHAT_HPP
#define __GNET_WORLDCHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class WorldChat : public GNET::Protocol
{
	#include "worldchat"
	class DispatchData
	{
		const WorldChat* chatmsg;
	public:
		DispatchData(const WorldChat* msg):chatmsg(msg) {}
		~DispatchData() {}
		void operator() (std::pair<const int, GNET::RoleData> pair)
		{
			if (pair.second.status == _STATUS_ONGAME)
				GLinkServer::GetInstance()->Send(pair.second.sid,chatmsg);
		}	
	};
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm = GLinkServer::GetInstance();
		if ( lsm->roleinfomap.size() )
			std::for_each(lsm->roleinfomap.begin(),lsm->roleinfomap.end(),DispatchData(this));
	}
};

};

#endif
