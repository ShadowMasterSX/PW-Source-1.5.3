
#ifndef __GNET_AUADDUPMONEYQUERY_RE_HPP
#define __GNET_AUADDUPMONEYQUERY_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void OnAuAddupMoneyQuery(int roleid,int64_t addupmoney);

namespace GNET
{

class AuAddupMoneyQuery_Re : public GNET::Protocol
{
	#include "auaddupmoneyquery_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		OnAuAddupMoneyQuery(roleid,addupmoney);
	}
};

};

#endif
