
#ifndef __GNET_ACQUESTION_HPP
#define __GNET_ACQUESTION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"

namespace GNET
{

class ACQuestion : public GNET::Protocol
{
	#include "acquestion"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		bool ret=true;
		unsigned int localsid=0;
		{
			GLinkServer::RoleMap::iterator it=lsm->roleinfomap.find(roleid);
			if ( it!=lsm->roleinfomap.end() && (*it).second.roleid==roleid && (*it).second.status==_STATUS_ONGAME )
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
			Log::log(LOG_DEBUG,"glinkd::acquestion :: send to roleid=%d failed, sid=%d, osbytes=%d",
				roleid, localsid, osbytes);
		}
	}
};

};

#endif
