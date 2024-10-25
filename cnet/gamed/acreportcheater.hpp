
#ifndef __GNET_ACREPORTCHEATER_HPP
#define __GNET_ACREPORTCHEATER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gsp_if.h"

namespace GNET
{

class ACReportCheater : public GNET::Protocol
{
	#include "acreportcheater"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if( cheattype <= 0 )
            acquestion_ret(roleid, -cheattype);
        else
            report_cheater(roleid, cheattype, cheatinfo.begin(), cheatinfo.size());
	}
};

};

#endif
