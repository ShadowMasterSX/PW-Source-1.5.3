
#include "gamedbclient.hpp"
#include "gdeliveryserver.hpp" //for zoneid,aid

#include "state.hxx"
#include "timertask.h"
#include "log.h"
#include "dbauctionlist.hrp"
#include "announcezoneid.hpp"
#include "dbsyncsellinfo.hrp"
#include "dbbattleload.hrp"
#include "dbpshopload.hrp"
#include "stockexchange.h"
#include "webtrademarket.h"
#include "factionfortressmanager.h"
#include "forcemanager.h"
#include "globalcontrol.h"
#include "disabled_system.h"
#include "namemanager.h"
#include "uniquedataserver.h"
#include "kingelection.h"
#include "announcecentraldelivery.hpp"
#include "centraldeliveryserver.hpp"
#include "mappasswd.h"
#include "dbmnfactioninfoget.hrp"
#include "dbmnfactionapplyinfoget.hrp"
#include "solochallengerank.h"

namespace GNET
{

GameDBClient GameDBClient::instance;

void GameDBClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	//backoff *= 2;
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* GameDBClient::GetInitState() const
{
	return &state_GameDBClient;
}

void GameDBClient::OnAddSession(Session::ID sid)
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
	// get auction list
	if(!DisabledSystem::GetDisabled(SYS_AUCTION)) Send(sid,Rpc::Call(RPC_DBAUCTIONLIST,DBAuctionListArg()));
	// get webtrade list
	if(!DisabledSystem::GetDisabled(SYS_WEBTRADE)) WebTradeMarket::GetInstance().OnDBConnect(this, sid);
	// get faction fortress
	if(!DisabledSystem::GetDisabled(SYS_FACTIONFORTRESS)) FactionFortressMan::GetInstance().OnDBConnect(this, sid);
	// get playershop list
	if(!DisabledSystem::GetDisabled(SYS_PLAYERSHOP)) Send(sid,Rpc::Call(RPC_DBPSHOPLOAD,DBPShopLoadArg()));
	// get force data
	ForceManager::GetInstance()->OnDBConnect(this, sid);
	KingElection::GetInstance().OnDBConnect(this, sid);

	if(!DisabledSystem::GetDisabled(SYS_BATTLE)) BattleManager::GetInstance()->OnDBConnect(this, sid);
	if(!DisabledSystem::GetDisabled(SYS_STOCK)) StockExchange::Instance()->OnDBConnect(this, sid);
	// send zoneid,aid, to announce self as Delivery to gamedbd
	GDeliveryServer* dsm=GDeliveryServer::GetInstance();
	Send( sid,AnnounceZoneid(dsm->zoneid,dsm->aid) );

	//Anounce whether central delivery or not to gamedbd
	AnnounceCentralDelivery acd_proto;
	acd_proto.is_central = dsm->IsCentralDS();
//	if(acd_proto.is_central)
//	{
//		CentralDeliveryServer* cds = CentralDeliveryServer::GetInstance();
//		cds->GetAcceptedZone(acd_proto.accepted_zone_list); //todo ddr
//	}
	Send(sid, acd_proto);

	// get sell point list
	Send( sid,Rpc::Call(RPC_DBSYNCSELLINFO,RoleId((unsigned int)-1)) );
	GlobalControl::GetInstance()->OnDBConnect(this, sid);
	if(!DisabledSystem::GetDisabled(SYS_UNIQUEDATAMAN)) UniqueDataServer::GetInstance()->OnDBConnect(this, sid);
    if (!DisabledSystem::GetDisabled(SYS_MAPPASSWORD)) Passwd::GetInstance().OnDBConnect(this, sid);
    if (!DisabledSystem::GetDisabled(SYS_SOLOCHALLENGERANK)) SoloChallengeRank::GetInstance().OnDBConnect(this, sid);
	//get rolename list
	NameManager::GetInstance()->OnDBConnect(this, sid);

	/*
	if(!DisabledSystem::GetDisabled(SYS_MNFACTIONBATTLE))
	{
		bool is_central = dsm->IsCentralDS();
		if(is_central)
		{
			Send(sid, Rpc::Call(RPC_DBMNFACTIONINFOGET, DBMNFactionInfoGetArg()));
			//Send(sid, Rpc::Call(RPC_DBMNFACTIONAPPLYINFOGET, DBMNFactionApplyInfoGetArg()));
		}
		else
		{
			Send(sid, Rpc::Call(RPC_DBMNFACTIONAPPLYINFOGET, DBMNFactionApplyInfoGetArg()));
		}
	}
	*/
}

void GameDBClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	Log::log(LOG_ERR,"gdelivery:: disconnect from GameDB\n");
}

void GameDBClient::OnAbortSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	Log::log(LOG_ERR,"gdelivery:: connect GameDB failed\n");
}

void GameDBClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

};
