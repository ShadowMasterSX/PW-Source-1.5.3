
#ifndef __GNET_GMTOGGLECHAT_HPP
#define __GNET_GMTOGGLECHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "gmtogglechat_re.hpp"
#include "mapuser.h"
#include "gamemaster.h"
namespace GNET
{

class GMToggleChat : public GNET::Protocol
{
	#include "gmtogglechat"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(MasterContainer::Instance().SetSilent(gmroleid, !enable))
			GDeliveryServer::GetInstance()->Send(sid,GMToggleChat_Re(ERR_SUCCESS,gmroleid,localsid,enable));
	}
};

};

#endif
