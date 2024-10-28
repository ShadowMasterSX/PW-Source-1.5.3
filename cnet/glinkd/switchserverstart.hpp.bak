
#ifndef __GNET_SWITCHSERVERSTART_HPP
#define __GNET_SWITCHSERVERSTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gproviderserver.hpp"
#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "log.h"

#include "switchservercancel.hpp"
#include "playerstatussync.hpp"
namespace GNET
{
class SwitchServerTimer : public Thread::Runnable
{
	unsigned int player_sid;
	int src_gsid;
	int dst_gsid;
public:
	SwitchServerTimer(unsigned int _psid, int _src_gsid, int _dst_gsid, int proirity=1) : Runnable(proirity), player_sid(_psid), src_gsid(_src_gsid), dst_gsid(_dst_gsid) { }
	void Run()
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		RoleData ui;
		if (lsm->GetSwitchUser(player_sid,ui) && this==ui.switch_flag)
		{
			lsm->PopSwitchUser(ui);
			//link server timeout, disconnect client
			Log::log(LOG_ERR,"glinkd:LinkServer timer out when switching user.user(r:%d,sid:%d),srcgsid(%d),dstgsid(%d)",ui.roleid,player_sid,src_gsid,dst_gsid);
			//lsm->SessionError(player_sid,ERR_TIMEOUT,"Server time out, when switching server.");
			lsm->Close(player_sid);
			lsm->ActiveCloseSession(player_sid);
		}
		delete this;
	}
};
class SwitchServerStart : public GNET::Protocol
{
	#include "switchserverstart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//ÎªÐÞ¸´¸´ÖÆbug£¬¸ÃÐ­Òé·¢ËÍÁ÷³ÌÐÞ¸ÄÎªgs->gdeliveryd->glinkd
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (!lsm->ValidRole(localsid,roleid))
		{
			//send playerstatusSync to gameserver
			GProviderServer::GetInstance()->DispatchProtocol(src_gsid,PlayerStatusSync(roleid,link_id,localsid,_STATUS_OFFLINE,src_gsid));

			Log::log(LOG_ERR,"glinkd::SwitchServerStart::invalid roleinfo(roleid=%d,localsid=%d).",roleid,localsid);
		   	return;
		}
		
		RoleData ui;
		ui=lsm->roleinfomap[roleid];
		if (lsm->IsInSwitch(ui))
		{
			Log::log(LOG_ERR,"glinkd::SwitchServerStart::Role(roleid=%d,localsid=%d) is already in switch state.",roleid,localsid);
		   	return; 
		}
		
#ifdef ANTI_TWIN_SYSTEM
		{
			Thread::Mutex::Scoped l(lsm->m_WindowsMapMutex);
			
	        SessionInfo* info = lsm->GetSessionInfo(ui.sid);
	        if (info->m_IsWindowRegistered == true)
	        {
	            uint32_t src_id = src_gsid;
	            //if (src_gsid == 84 || src_gsid == 85 || src_gsid == 86) //Special for Dynasty Battle;
	                //src_id = 83;
	            GLinkServer::WindowsMap::iterator it = lsm->m_WindowsMap.find(src_id);
	            if (it != lsm->m_WindowsMap.end())
	            {
	                GLinkServer::WindowPog& srcInst = it->second;
	                //lsm->m_WindowsMapMutex.Lock();
	                GLinkServer::WindowPog::iterator instIt = srcInst.find(info->m_MachineHash);
	                //lsm->m_WindowsMapMutex.UNLock();
	                if (instIt != srcInst.end())
	                {
	                    //lsm->m_WindowsMapMutex.Lock();
	                    --srcInst[info->m_MachineHash];
	                    //lsm->m_WindowsMapMutex.UNLock();
	                    info->m_IsWindowRegistered = false;
	                    printf("Delete window! count=%d, inst_id=%d, sid=%d\n", srcInst[info->m_MachineHash], src_gsid, ui.sid);
	                }
	            }
	        }

	        if (info->m_IsWindowRegistered == false)
	        {
	            uint32_t maxWindowsCount = 1;
	            uint32_t dst_id = dst_gsid;
	            //if (dst_gsid == 84 || dst_gsid == 85 || dst_gsid == 86) //Special for Dynasty Battle;
	                //dst_id = 83;
	            GLinkServer::WindowsMap::iterator it = lsm->m_WindowsMap.find(dst_id);
	            if (it != lsm->m_WindowsMap.end())
	            {
	                if (info->m_MachineHash == 0)
	                {
	                    printf("Cancel switch location, empty hash! inst_id=%d, sid=%d\n", dst_gsid, ui.sid);
	                    GProviderServer::GetInstance()->DispatchProtocol(src_gsid, SwitchServerCancel(roleid, link_id, localsid));
	                    GDeliveryClient::GetInstance()->SendProtocol(SwitchServerCancel(roleid, link_id, localsid));

	                    Octets octet;
	                    uint8_t data[6] = { 0x19, 0x00, 0x2D, 0x00, 0x00, 0x00 };
	                    octet.insert(octet.end(), data, sizeof(data));
	                    lsm->AccumulateSend(ui.sid, GamedataSend(octet));
	                    return;
	                }

	                GLinkServer::WindowPog& dstInst = it->second;
	                //lsm->m_WindowsMapMutex.Lock();
	                GLinkServer::WindowPog::iterator instIt = dstInst.find(info->m_MachineHash);
	                //lsm->m_WindowsMapMutex.UNLock();
	                if (instIt == dstInst.end())
	                {
	                    //lsm->m_WindowsMapMutex.Lock();
	                    dstInst.insert( GLinkServer::WindowPog::value_type(info->m_MachineHash, 1) );
	                    //lsm->m_WindowsMapMutex.UNLock();
	                    info->m_IsWindowRegistered = true;
	                    printf("Register window! count=1, inst_id=%d, sid=%d\n", dst_gsid, ui.sid);
	                }
	                else
	                {
	                    uint32_t windowsCount = dstInst[info->m_MachineHash];
	                    if (windowsCount + 1 > maxWindowsCount)//Ïðîâåðêà íà ìàêñèìàëüíîå êîë-âî îêîí.
	                    {
	                        printf("Cancel switch location, max windows! count=%u, inst_id=%d, sid=%d\n", windowsCount, dst_gsid, ui.sid);
	                        GProviderServer::GetInstance()->DispatchProtocol(src_gsid, SwitchServerCancel(roleid, link_id, localsid));
	                        GDeliveryClient::GetInstance()->SendProtocol(SwitchServerCancel(roleid, link_id, localsid));

	                        Octets octet;
	                        uint8_t data[6] = { 0x19, 0x00, 0xFE, 0x29, 0x00, 0x00 };
	                        octet.insert(octet.end(), data, sizeof(data));
	                        lsm->AccumulateSend(ui.sid, GamedataSend(octet));
	                        return;
	                    }
	                    else
	                    {
	                        //lsm->m_WindowsMapMutex.Lock();
	                        ++dstInst[info->m_MachineHash];
	                        //lsm->m_WindowsMapMutex.UNLock();
	                        info->m_IsWindowRegistered = true;
	                        printf("Register window! count=%d, inst_id=%d, sid=%d\n", dstInst[info->m_MachineHash], dst_gsid, ui.sid);
	                    }
	                }
	            }
	        }
	    }
#endif //ANTI_TWIN_SYSTEM
		if (!GProviderServer::GetInstance()->DispatchProtocol(dst_gsid,this))
		{
			Log::log(LOG_WARNING,"glinkd::SwitchServerStart:: dst_gsid(%d) is not exist or disconnect from linkserver.",dst_gsid);
			//send switch_cancel to source gameserver
			GProviderServer::GetInstance()->DispatchProtocol(src_gsid,SwitchServerCancel(roleid,link_id,localsid));
			//send switch_cancel to gdelivery server
			GDeliveryClient::GetInstance()->SendProtocol(SwitchServerCancel(roleid,link_id,localsid));
			return;
		} 
		ui.gs_id = dst_gsid;	//save dst_gsid to switch user map
		ui.switch_flag=new SwitchServerTimer(ui.sid,src_gsid,dst_gsid);
		lsm->PushSwitchUser(ui);

		Thread::HouseKeeper::AddTimerTask( (SwitchServerTimer*)ui.switch_flag,15 ); //set 15s to timeout
	}
};

};

#endif
