
#ifndef __GNET_POSTEXPIRE_RE_HPP
#define __GNET_POSTEXPIRE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PostExpire_Re : public GNET::Protocol
{
	#include "postexpire_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
