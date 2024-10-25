
#ifndef __GNET_ANNOUNCELINKTYPE_HPP
#define __GNET_ANNOUNCELINKTYPE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gdeliveryserver.hpp"
namespace GNET
{

class AnnounceLinkType : public GNET::Protocol
{
	#include "announcelinktype"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (LINK_TYPE_IWEB == link_type)
			GDeliveryServer::GetInstance()->iweb_sid = sid;
		GDeliveryServer::GetInstance()->InsertLinkType(sid, link_type);
		LOG_TRACE("AnnounceLinkType sid=%d link_type=%d",sid,link_type);
	}
};

};

#endif
