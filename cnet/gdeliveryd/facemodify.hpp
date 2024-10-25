
#ifndef __GNET_FACEMODIFY_HPP
#define __GNET_FACEMODIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gdeliveryserver.hpp"

#include "mapticket.h"

namespace GNET
{

class FaceModify : public GNET::Protocol
{
	#include "facemodify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		FaceTicketSet::GetInstance().AddTicket( roleid, FaceTicketSet::face_ticket_t(ticket_id,ticket_pos) );
	}
};

};

#endif
