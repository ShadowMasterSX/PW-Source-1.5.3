
#ifndef __GNET_ACREPORT_HPP
#define __GNET_ACREPORT_HPP

#include "gacdutil.h"
#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "reportinfo.hpp"

namespace GNET
{

class ACReport : public GNET::Protocol
{
	#include "acreport"

	void Process(Manager *manager, Manager::Session::ID sid)
	{

		DEBUG_PRINT_CLIENTINFO("acreport recieved from user %d\n", roleid);

		ReportInfo::DeliverReport(roleid, report);
	}
};

};

#endif
