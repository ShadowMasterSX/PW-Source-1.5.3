
#ifndef __GNET_FACTIONRESOURCEBATTLESTATUSNOTICE_HPP
#define __GNET_FACTIONRESOURCEBATTLESTATUSNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


void notify_mafia_pvp_status(int status,std::vector<int> &ctrl_list);
namespace GNET
{

class FactionResourceBattleStatusNotice : public GNET::Protocol
{
	#include "factionresourcebattlestatusnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		notify_mafia_pvp_status(status,controller_list);
	}
};

};

#endif
