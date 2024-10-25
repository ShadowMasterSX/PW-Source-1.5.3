
#ifndef __GNET_QPGETACTIVATEDSERVICES_HPP
#define __GNET_QPGETACTIVATEDSERVICES_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mapuser.h"
#include "game2au.hpp"

namespace GNET
{

class QPGetActivatedServices : public GNET::Protocol
{
	#include "qpgetactivatedservices"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		UserInfo* ui = UserContainer::GetInstance().FindUser(userid);
		if (ui == NULL || ui->linksid != sid)
		{
			return;
		}

		Game2AU proto;
		proto.userid = userid;
		proto.qtype = Game2AU::GET_ACTIVATED_MERCHANTS;

		GAuthClient::GetInstance()->SendProtocol( proto );
	}
};

};

#endif
