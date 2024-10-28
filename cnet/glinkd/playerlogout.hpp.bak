
#ifndef __GNET_PLAYERLOGOUT_HPP
#define __GNET_PLAYERLOGOUT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "statusannounce.hpp"
namespace GNET
{

class PlayerLogout : public GNET::Protocol
{
	#include "playerlogout"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm = GLinkServer::GetInstance();

		SessionInfo * sinfo = lsm->GetSessionInfo(localsid);
		if (!sinfo || sinfo->roleid!=roleid)
			return;
		
		
		
#ifdef ANTI_TWIN_SYSTEM
        if (sinfo->m_IsWindowRegistered == true)
        {
        	Thread::Mutex::Scoped l(lsm->m_WindowsMapMutex);
        	
            uint32_t src_id = sinfo->gsid;
            if (sinfo->gsid == 84 || sinfo->gsid == 85 || sinfo->gsid == 86) //Special for Dynasty Battle;
                src_id = 83;
            GLinkServer::WindowsMap::iterator it = lsm->m_WindowsMap.find(src_id);
            if (it != lsm->m_WindowsMap.end())
            {
                uint64_t hash = sinfo->m_MachineHash;
                GLinkServer::WindowPog& srcInst = it->second;
                lsm->m_WindowsMapMutex.Lock();
                GLinkServer::WindowPog::iterator instIt = srcInst.find(hash);
                lsm->m_WindowsMapMutex.UNLock();
                if (instIt != srcInst.end())
                {
                    lsm->m_WindowsMapMutex.Lock();
                    --srcInst[hash];
                    lsm->m_WindowsMapMutex.UNLock();
                    sinfo->m_IsWindowRegistered = false;
                    printf("Delete window! count=%d, inst_id=%d, sid=%d\n", srcInst[hash], sinfo->gsid, localsid);
                }
            }
        }
#endif //ANTI_TWIN_SYSTEM
		
		
		sinfo->gsid   = 0;
		sinfo->roleid = 0;

		lsm->AccumulateSend(localsid,this);
		lsm->RoleLogout(roleid);
		//change state of linkserver
		if (result==_PLAYER_LOGOUT_FULL)
		{
			// todo check intention of readyclosetime
			lsm->SetReadyCloseTime(localsid, 30);
		}
	}
};

};

#endif
