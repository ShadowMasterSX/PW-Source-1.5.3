
#ifndef __GNET_UNIQUEDATAMODIFYBROADCAST_HPP
#define __GNET_UNIQUEDATAMODIFYBROADCAST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "guniquedataelemnode"

namespace GNET
{

class UniqueDataModifyBroadcast : public GNET::Protocol
{
	#include "uniquedatamodifybroadcast"
	class DispatchData
	{
		const UniqueDataModifyBroadcast* modifymsg;
	public:
		DispatchData(const UniqueDataModifyBroadcast* msg): modifymsg(msg) {}
		~DispatchData() {}
		void operator() (std::pair<const int, GNET::RoleData> pair)
		{
			if (pair.second.status == _STATUS_ONGAME)
				GLinkServer::GetInstance()->Send(pair.second.sid,modifymsg);
		}	
	};

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkServer* lsm=GLinkServer::GetInstance();
		if ( lsm->roleinfomap.size() )
			std::for_each(lsm->roleinfomap.begin(),lsm->roleinfomap.end(),DispatchData(this));
	}
};

};

#endif
