
#ifndef __GNET_ANNOUNCELINKVERSION_HPP
#define __GNET_ANNOUNCELINKVERSION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AnnounceLinkVersion : public GNET::Protocol
{
	#include "announcelinkversion"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
