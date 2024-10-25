
#ifndef __GNET_GAMEDATARESP_HPP
#define __GNET_GAMEDATARESP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GameDataResp : public GNET::Protocol
{
	#include "gamedataresp"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
