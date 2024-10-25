
#ifndef __GNET_FACEMODIFYCANCEL_HPP
#define __GNET_FACEMODIFYCANCEL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gdeliveryserver.hpp"

#include "mapticket.h"

namespace GNET
{

class FaceModifyCancel : public GNET::Protocol
{
	#include "facemodifycancel"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		FaceTicketSet::GetInstance().RemoveTicket( roleid );
	}
};

};

#endif
