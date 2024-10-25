
#ifndef __GNET_FACTIONFORTRESSENTERNOTICE_HPP
#define __GNET_FACTIONFORTRESSENTERNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class FactionFortressEnterNotice : public GNET::Protocol
{
	#include "factionfortressenternotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
