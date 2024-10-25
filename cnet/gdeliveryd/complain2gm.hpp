
#ifndef __GNET_COMPLAIN2GM_HPP
#define __GNET_COMPLAIN2GM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gdeliveryserver.hpp"
#include "complain2gm_re.hpp"
namespace GNET
{

class Complain2GM : public GNET::Protocol
{
	#include "complain2gm"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		this->zoneid=dsm->zoneid;
		if ( dsm->Send(dsm->iweb_sid, this) )
		{
			dsm->Send(sid,Complain2GM_Re(ERR_SUCCESS,roleid,localsid));
		}
		else
			dsm->Send(sid,Complain2GM_Re(ERR_COMMUNICATION,roleid,localsid));	
	}
};

};

#endif
