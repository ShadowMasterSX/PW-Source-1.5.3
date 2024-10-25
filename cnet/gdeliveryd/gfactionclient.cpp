
#include "gfactionclient.hpp"
#include "state.hxx"
#include "timertask.h"
#include "gdeliveryserver.hpp"
#include "playerstatusannounce.hpp"
#include "gproviderserver.hpp"

#include "announcezoneid.hpp"
#include "announceproviderid.hpp"
#include "mapuser.h"
#include "battlemanager.h"
#include "factionfortressmanager.h"
#include "announcecentraldelivery.hpp"
#include "factionresourcebattleman.h"

namespace GNET
{

GFactionClient GFactionClient::instance;

void GFactionClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	//backoff *= 2;
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* GFactionClient::GetInitState() const
{
	return &state_GFactionDeliveryClient;
}

class FactionClientQueryUser : public UserContainer::IQueryUser
{   
public:
	PlayerStatusAnnounce psa;
	bool Update( int userid, UserInfo & info )
	{
		psa.status=_STATUS_ONGAME;
		if (info.status==_STATUS_ONGAME)
		{
			psa.playerstatus_list.add(OnlinePlayerStatus(info.roleid,info.gameid,info.linkid,info.localsid));
		}
		return true;
	}
};

void GFactionClient::OnAddSession(Session::ID sid)
{
	{
		Thread::Mutex::Scoped l(locker_state);
		if (conn_state)
		{
			Close(sid);
			return;
		}
		conn_state = true;
		this->sid = sid;
		backoff = BACKOFF_INIT;
	} //end of state locker
	DEBUG_PRINT("gdelivery::connect gfactionserver successfully.\n");
	//TODO
	//Announce DeliveryServer's ID to gFactionServer
	SendProtocol( AnnounceProviderID((int)GProviderServer::GetInstance()->GetProviderServerID()) );
	//Announce DeliveryServer's zoneid to gFactionServer
	SendProtocol( AnnounceZoneid(GDeliveryServer::GetInstance()->zoneid) );
	
	//Anounce whether central delivery or not to gFactionServer
	AnnounceCentralDelivery acd_proto;
	acd_proto.is_central = GDeliveryServer::GetInstance()->IsCentralDS();
	Send(sid, acd_proto);

	//Announce all online users to gFactionServer
	FactionClientQueryUser	q;
	UserContainer::GetInstance().WalkUser( q );
	if (q.psa.playerstatus_list.size() > 0)
		SendProtocol(q.psa);
	//通知gfaction加载城战相关帮派信息
	BattleManager::GetInstance()->SyncBattleFaction();
	//通知gfaction加载基地相关帮派信息
	FactionFortressMan::GetInstance().SyncFactionServer();
    FactionResourceBattleMan::GetInstance()->SyncAllFactionBattleInfo();
}

void GFactionClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	DEBUG_PRINT("gdelivery::gfactionclient: disconnect from server. reconnecting...\n");
}

//void GFactionClient::OnAbortSession(const SockAddr &sa)
void GFactionClient::OnAbortSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	DEBUG_PRINT("gdelivery::gfactionclient: connect server failed. reconnecting...\n");
}

void GFactionClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

};
