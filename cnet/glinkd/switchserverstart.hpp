
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
		//为修复复制bug，该协议发送流程修改为gs->gdeliveryd->glinkd
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
