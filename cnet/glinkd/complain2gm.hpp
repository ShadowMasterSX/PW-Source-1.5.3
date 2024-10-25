
#ifndef __GNET_COMPLAIN2GM_HPP
#define __GNET_COMPLAIN2GM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "complain2gm_re.hpp"
namespace GNET
{

class Complain2GM : public GNET::Protocol
{
	#include "complain2gm"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		if ( !lsm->ValidRole(sid,roleid) )
			return;
		if ( lsm->forbidcomplaincache.get(roleid) )
		{
			lsm->Send(sid,Report2GM_Re(ERR_GENERAL,roleid,_SID_INVALID));
			return;
		}
		localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
		lsm->forbidcomplaincache.put(std::make_pair(roleid,roleid));
	}
};

};

#endif
