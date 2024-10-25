
#ifndef __GNET_FACTIONINFOREQ_HPP
#define __GNET_FACTIONINFOREQ_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "dbgametalkfactioninfo.hrp"

namespace GNET
{

class FactionInfoReq : public GNET::Protocol
{
	#include "factioninforeq"

	void Process(Manager *manager, Manager::Session::ID sid)
	{

		DBGameTalkFactionInfoArg arg;
		arg.factionid = (unsigned int)(factionid.factionid);
		DBGameTalkFactionInfo *rpc = (DBGameTalkFactionInfo *)Rpc::Call(RPC_DBGAMETALKFACTIONINFO, arg);
		rpc->localuid = localuid;
		rpc->factionid = factionid;
		rpc->save_manager = manager; 
		rpc->save_sid = sid;
		GameDBClient::GetInstance()->SendProtocol(rpc);
	}
};

};

#endif
