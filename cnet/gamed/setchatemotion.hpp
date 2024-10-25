
#ifndef __GNET_SETCHATEMOTION_HPP
#define __GNET_SETCHATEMOTION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class SetChatEmotion : public GNET::Protocol
{
	#include "setchatemotion"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
