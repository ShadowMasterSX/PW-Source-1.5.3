
#ifndef __GNET_WEBPOSTCANCEL_RE_HPP
#define __GNET_WEBPOSTCANCEL_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class WebPostCancel_Re : public GNET::Protocol
{
	#include "webpostcancel_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
