
#ifndef __GNET_ACREPORTCHEATER_HPP
#define __GNET_ACREPORTCHEATER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class ACReportCheater : public GNET::Protocol
{
	#include "acreportcheater"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
