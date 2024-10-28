
#ifndef __GNET_COLLECTCLIENTMACHINEINFO_HPP
#define __GNET_COLLECTCLIENTMACHINEINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryclient.hpp"

#include "glinkserver.hpp"

namespace GNET
{

class CollectClientMachineInfo : public GNET::Protocol
{
	#include "collectclientmachineinfo"
	
    static inline uint64_t Hash(uint8_t* a_Buffer, uint32_t a_BufferLen)
    {
    	//uint64_t h = 1125899906842597L; // prime
        uint64_t h = 600851475143ULL; // prime
        for (size_t i = 0; i < a_BufferLen; ++i)
            h = 31 * h + a_Buffer[i];
        return h;
    }
	
	void Process(Manager *manager, Manager::Session::ID sid)
	{
        GDeliveryClient::GetInstance()->SendProtocol(this);
        
        // Modifed by Apfilipp Ч START
        if (!GLinkServer::ValidUser(sid, userid))
            return;

        uint8_t key[4] = { 0x34, 0x56, 0x87, 0x48 };

        Octets data;
        data.insert(data.end(), key, sizeof(key));

        ARCFourSecurity rc4;
        rc4.SetParameter(data);
        rc4.Update(machineinfo);
        if (strstr((const char*)machineinfo.begin(), "0:0|") == NULL)
            return;

        SessionInfo* info = GLinkServer::GetInstance()->GetSessionInfo(sid);
        if (info != NULL)
        {
            printf("ClientMachineInfo info=%s, sid=%d\n", (const char*)machineinfo.begin(), sid);
            uint64_t hash = Hash((uint8_t*)machineinfo.begin(), machineinfo.size());
            info->m_MachineHash = hash;
#ifdef ANTI_TWIN_SYSTEM
            GLinkServer* lsm = GLinkServer::GetInstance();
            uint32_t dst_id = info->gsid;
            //if (info->gsid == 84 || info->gsid == 85 || info->gsid == 86) //Special for Dynasty Battle;
                //dst_id = 83;
            Thread::Mutex::Scoped l(lsm->m_WindowsMapMutex);
            GLinkServer::WindowsMap::iterator it = lsm->m_WindowsMap.find(dst_id);
            if (it != lsm->m_WindowsMap.end())
            {
                GLinkServer::WindowPog& dstInst = it->second;
                //lsm->m_WindowsMapMutex.Lock();
                GLinkServer::WindowPog::iterator instIt = dstInst.find(hash);
                //lsm->m_WindowsMapMutex.UNLock();
                if (instIt == dstInst.end())
                {
                    //lsm->m_WindowsMapMutex.Lock();
                    dstInst.insert( GLinkServer::WindowPog::value_type(hash, 1)/*std::pair<uint64_t, uint32_t>(hash, 1)*/ );
                    //lsm->m_WindowsMapMutex.UNLock();
                    info->m_IsWindowRegistered = true;
                    printf("Register window! count=1, inst_id=%d, sid=%d\n", info->gsid, sid);
                }
                else
                {
                    uint32_t windowsCount = dstInst[hash];
                    if (windowsCount + 1 > 1)//ѕроверка на максимальное кол-во окон.
                    {
#ifdef DEBUG_PERMISSION_SYSTEM
                        printf("Teleport from location, max windows! count=%u, inst_id=%d, sid=%d\n", windowsCount, info->gsid, sid);
                        Octets data;
                        uint8_t tp[18] = { 0xB8, 0x22, 0x01, 0x00, 0x00, 0x00, 0xCD, 0xF4, 0xFF, 0xFF, 0x8E, 0xF4, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00 };
                        data.insert(data.end(), tp, sizeof(tp));
                        GProviderServer::GetInstance()->DispatchProtocol(info->gsid, C2SGamedataSend(info->roleid, sid, data));
                        Octets octet;
                        uint8_t error[6] = { 0x19, 0x00, 0xFE, 0x29, 0x00, 0x00 };
                        octet.insert(octet.end(), error, sizeof(error));
                        lsm->AccumulateSend(sid, GamedataSend(octet));
                        return;
#else
                        printf("Disconnect from location, max windows! count=%u, inst_id=%d, sid=%d\n", windowsCount, info->gsid, sid);
                        lsm->Close(sid);
                        lsm->ActiveCloseSession(sid);
#endif // DEBUG_PERMISSION_SYSTEM
                    }
                    else
                    {
                        //lsm->m_WindowsMapMutex.Lock();
                        ++dstInst[hash];
                        //lsm->m_WindowsMapMutex.UNLock();
                        info->m_IsWindowRegistered = true;
                        printf("Register window! count=%d, inst_id=%d, sid=%d\n", windowsCount + 1, info->gsid, sid);
                    }
                }
            }
#endif //ANTI_TWIN_SYSTEM
        }
        // Modifed by Apfilipp Ч END
	}
};

};

#endif
