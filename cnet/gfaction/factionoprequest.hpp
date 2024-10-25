
#ifndef __GNET_FACTIONOPREQUEST_HPP
#define __GNET_FACTIONOPREQUEST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "operwrapper.h"

namespace GNET
{

class FactionOPRequest : public GNET::Protocol
{
	#include "factionoprequest"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		unsigned int tid=OperWrapper::CreateWrapper(roleid,(GNET::Operations)optype);
		if (tid!=0/*invalid tid*/)
		{
			DEBUG_PRINT("Create OperWrapper,tid=%d,roleid=%d,optype=%d\n",tid,roleid,optype);
			OperWrapper::href_t op=OperWrapper::href_t( OperWrapper::FindWrapper(tid) );
			if ( op )
			{
				op->SetParams(params);
				op->Execute();
			}
		}
		else
			Log::log(LOG_ERR,"Create OperWrapper failed.tid=%d,roleid=%d,optype=%d\n",tid,roleid,optype);
	}
};

};

#endif
