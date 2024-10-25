
#ifndef __GNET_ANNOUNCEFORBIDINFO_HPP
#define __GNET_ANNOUNCEFORBIDINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class AnnounceForbidInfo : public GNET::Protocol
{
	#include "announceforbidinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
