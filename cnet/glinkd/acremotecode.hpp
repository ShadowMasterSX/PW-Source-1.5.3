
#ifndef __GNET_ACREMOTECODE_HPP
#define __GNET_ACREMOTECODE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
namespace GNET
{

class ACRemoteCode : public GNET::Protocol
{
	#include "acremotecode"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		bool ret=true;
		unsigned int localsid=0;
		{
			GLinkServer::RoleMap::iterator it=lsm->roleinfomap.find(dstroleid);
			if ( it!=lsm->roleinfomap.end() && (*it).second.roleid==dstroleid && (*it).second.status==_STATUS_ONGAME )
			{
				if ( !lsm->Send((*it).second.sid, this, true) )
				{
					ret=false;
					localsid=(*it).second.sid;
				}
			}
		}
		if ( !ret )
		{
			size_t osbytes = lsm->GetOsbytes(localsid);
			lsm->Close( localsid );
			lsm->ActiveCloseSession( localsid );
			Log::log(LOG_DEBUG,"glinkd::acremotecode :: send to roleid=%d failed, sid=%d, osbytes=%d",
				dstroleid, localsid, osbytes);
		}
	}
};

};

#endif
