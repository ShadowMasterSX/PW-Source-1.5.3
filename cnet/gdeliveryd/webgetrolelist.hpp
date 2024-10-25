
#ifndef __GNET_WEBGETROLELIST_HPP
#define __GNET_WEBGETROLELIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "dbwebtradegetrolesimpleinfo.hrp"

namespace GNET
{

class WebGetRoleList : public GNET::Protocol
{
	#include "webgetrolelist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("webgetrolelist: receive. userid=%d\n", userid); 
	
		DBWebTradeGetRoleSimpleInfo * rpc = (DBWebTradeGetRoleSimpleInfo *)Rpc::Call(
			RPC_DBWEBTRADEGETROLESIMPLEINFO,
			DBWebTradeGetRoleSimpleInfoArg(userid)		
		);
		rpc->aid = GDeliveryServer::GetInstance()->aid; 
		rpc->messageid = messageid;
		rpc->timestamp = timestamp;
		GameDBClient::GetInstance()->SendProtocol(rpc);
	}
};

};

#endif
