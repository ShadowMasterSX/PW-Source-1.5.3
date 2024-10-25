
#ifndef __GNET_COUNTRYBATTLESINGLEBATTLERESULT_HPP
#define __GNET_COUNTRYBATTLESINGLEBATTLERESULT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleSingleBattleResult : public GNET::Protocol
{
	#include "countrybattlesinglebattleresult"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid, this);
	}
};

};

#endif
