
#ifndef __GNET_QUERYUSERPRIVILEGE_RE_HPP
#define __GNET_QUERYUSERPRIVILEGE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class QueryUserPrivilege_Re : public GNET::Protocol
{
	#include "queryuserprivilege_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		printf("receive queryuserprivilege_re.\n");
		for (size_t i=0;i<auth.size();i++)
		{
			printf("=======user has privilege %d======\n",auth[i]);
		}
	}
};

};

#endif
