
#ifndef __GNET_QUERYUSERPRIVILEGE_HPP
#define __GNET_QUERYUSERPRIVILEGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "queryuserprivilege_re.hpp"
namespace GNET
{

class QueryUserPrivilege : public GNET::Protocol
{
	#include "queryuserprivilege"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		QueryUserPrivilege_Re qup_re;
		qup_re.userid=userid;
		qup_re.auth.add(0);
		qup_re.auth.add(1);
		qup_re.auth.add(2);
		qup_re.auth.add(3);
		qup_re.auth.add(4);
		qup_re.auth.add(5);
		qup_re.auth.add(6);
		qup_re.auth.add(7);
		qup_re.auth.add(8);
		qup_re.auth.add(9);
		qup_re.auth.add(10);
		qup_re.auth.add(11);

		qup_re.auth.add(100);
		qup_re.auth.add(101);
		qup_re.auth.add(102);
		qup_re.auth.add(103);
		qup_re.auth.add(104);
		qup_re.auth.add(105);

		qup_re.auth.add(200);
		qup_re.auth.add(201);
		qup_re.auth.add(202);
		qup_re.auth.add(203);
		qup_re.auth.add(204);
		qup_re.auth.add(205);
		qup_re.auth.add(206);

		manager->Send(sid,qup_re);
	}
};

};

#endif
