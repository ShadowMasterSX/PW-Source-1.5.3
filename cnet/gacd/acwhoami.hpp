
#ifndef __GNET_ACWHOAMI_HPP
#define __GNET_ACWHOAMI_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "ganticheaterserver.hpp"
#include "gaccontrolserver.hpp"

namespace GNET
{

class ACWhoAmI : public GNET::Protocol
{
	#include "acwhoami"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if( clienttype == _DELIVERY_CLIENT )
			GAntiCheaterServer::GetInstance()->SetDeliverSID(sid);
		else if( clienttype == _CONTROL_CLIENT )
			GACControlServer::GetInstance()->SetControlSID(sid);
	}
};

};

#endif
