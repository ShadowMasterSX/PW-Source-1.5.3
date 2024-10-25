
#ifndef __GNET_ANNOUNCELINKTYPE_HPP
#define __GNET_ANNOUNCELINKTYPE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AnnounceLinkType : public GNET::Protocol
{
	#include "announcelinktype"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
