#include "gdeliveryserver.hpp"
#include "gauthclient.hpp"
#include "gproviderserver.hpp"
#include "gamedbclient.hpp"
#include "groledbclient.hpp"
#include "gfactionclient.hpp"
#include "ganticheatclient.hpp"
#include "uniquenameclient.hpp"
#include "gwebtradeclient.hpp"
#include "gametalkclient.hpp"
#include "gametalkmanager.h"
#include "snsclient.hpp"
#include "conf.h"
#include "log.h"
#include "thread.h"
#include <iostream>
#include <unistd.h>
#include <time.h>
#include "itimer.h"
#include "deleteroletask.h"
#include "matcher.h"
#include "mapforbid.h"
#include "mapuser.h"
#include "mapgameattr.h"
#include "battlemanager.h"
#include "auctionmarket.h"
#include "billingagent.h"
#include "serverattr.h"
#include "stockexchange.h"
#include "xmlversion.h"
#include "referencemanager.h"
#include "rewardmanager.h"
#include "webtrademarket.h"
#include "sysauctionmanager.h"
#include "factionfortressmanager.h"
#include "forcemanager.h"
#include "friendextinfomanager.h"
#include "countrybattleman.h"
#include "disabled_system.h"
#include "kingelection.h"
#include "pshopmarket.h"
#include "gdeliverytool.h"
#include "playerprofileman.h"
#include "autoteamman.h"
#include "uniquedataserver.h"
#include "tankbattlemanager.h"
#include "factionresourcebattleman.h"
#include "mappasswd.h"

#include "centraldeliveryserver.hpp"
#include "centraldeliveryclient.hpp"

using namespace GNET;

bool CheckIPAddress(const char * ipstr)
{
	if(ipstr == NULL) return false;
	struct in_addr ip;
	if(!inet_aton(ipstr, &ip)) return false;
	return ip.s_addr != INADDR_ANY && ip.s_addr != INADDR_BROADCAST;
}

void usage(char* name)
{
	printf ("Usage: %s [-v] [-h] [configurefile]\n", name);
}

int main(int argc, char *argv[])
{
	int opt;
	while((opt = getopt(argc, argv, "hv")) != EOF)
	{
		switch(opt)
		{
			case 'v':
				printf("Compiled By "__USER__ ", "__DATE__ " "__TIME__ "\n");
				printf("%s\n", XMLVERSION);
				exit(0);
			default:
				usage(argv[0]);
				exit(0);
		}
	}
	if (optind >= argc || access(argv[optind], R_OK) == -1)
	{
		usage(argv[0]);
		exit(-1);
	}
	
	srand(time(NULL));

	Conf *conf = Conf::GetInstance(argv[optind]);
	GDeliveryServer *manager = GDeliveryServer::GetInstance();
	
	std::string central_ds = conf->find(manager->Identification(), "is_central_ds");
	if (central_ds == "true") manager->SetCentralDS(true);
	bool is_central = manager->IsCentralDS();
		
	//根据配置信息设置disabledsystem
	//DisabledSystem::SetDisabled(INDEX);
	int battle = atoi(conf->find(GDeliveryServer::GetInstance()->Identification(), "battlefield").c_str());
	if(!battle) DisabledSystem::SetDisabled(SYS_BATTLE);
	int countrybattle = atoi(conf->find("COUNTRYBATTLE", "is_countrybattle_open").c_str());
	if(!countrybattle) DisabledSystem::SetDisabled(SYS_COUNTRYBATTLE);	
	int kingelection = atoi(conf->find("COUNTRYBATTLE", "kingelection_open").c_str());
	if(!kingelection) DisabledSystem::SetDisabled(SYS_KINGELECTION);	
	int recall = atoi(conf->find(GDeliveryServer::GetInstance()->Identification(), "recalloldplayer").c_str());
	if(!recall) DisabledSystem::SetDisabled(SYS_RECALLOLDPLAYER);
	int tankbattle = atoi(conf->find("TANKBATTLE", "is_tankbattle_open").c_str());
	if(!tankbattle) DisabledSystem::SetDisabled(SYS_TANKBATTLE);	
    int factionresourcebattle = atoi(conf->find("FACTIONRESOURCEBATTLE", "is_factionresourcebattle_open").c_str());
    if(!factionresourcebattle) DisabledSystem::SetDisabled(SYS_FACTIONRESOURCEBATTLE);	
	if(!CheckIPAddress(conf->find(GWebTradeClient::GetInstance()->Identification(), "address").c_str()))
		DisabledSystem::SetDisabled(SYS_WEBTRADE);
	if(!CheckIPAddress(conf->find(SNSClient::GetInstance()->Identification(), "address").c_str()))
		DisabledSystem::SetDisabled(SYS_SNS);
	if(!CheckIPAddress(conf->find(GameTalkClient::GetInstance()->Identification(), "address").c_str()))
		DisabledSystem::SetDisabled(SYS_GAMETALK);

	if(is_central) {
		DisabledSystem::SetDisabled(SYS_AUCTION);
		DisabledSystem::SetDisabled(SYS_STOCK);
		DisabledSystem::SetDisabled(SYS_BATTLE);
		DisabledSystem::SetDisabled(SYS_SYSAUCTION);
		DisabledSystem::SetDisabled(SYS_WEBTRADE);
		DisabledSystem::SetDisabled(SYS_GAMETALK);
		DisabledSystem::SetDisabled(SYS_SNS);
		DisabledSystem::SetDisabled(SYS_FACTIONFORTRESS);
		DisabledSystem::SetDisabled(SYS_REFERENCE);
		DisabledSystem::SetDisabled(SYS_REWARD);
		DisabledSystem::SetDisabled(SYS_RECALLOLDPLAYER);
		DisabledSystem::SetDisabled(SYS_KINGELECTION);
		DisabledSystem::SetDisabled(SYS_PLAYERSHOP);
		DisabledSystem::SetDisabled(SYS_PLAYERPROFILE);
		DisabledSystem::SetDisabled(SYS_AUTOTEAM);
		DisabledSystem::SetDisabled(SYS_UNIQUEDATAMAN);
        DisabledSystem::SetDisabled(SYS_FACTIONRESOURCEBATTLE);
        DisabledSystem::SetDisabled(SYS_MAPPASSWORD);
	}

	int ret = Matcher::GetInstance()->Load(const_cast<char*>(conf->find(manager->Identification(),"table_name").c_str()),
			"UCS2", conf->find(manager->Identification(),"name_charset").c_str(),
			conf->find(manager->Identification(), "table_charset").c_str());
	if(ret)
	{
		std::cerr<<"Cannot load table of sensitive words. check file ./filters, ret=" << ret << std::endl;
		exit(-1);
	}

	if(0 != DELIVERY_TOOL::OnFilterMode(argc,argv)) exit(-1);

	IntervalTimer::StartTimer(500000);
	if(!DisabledSystem::GetDisabled(SYS_STOCK) && !StockExchange::Instance()->Initialize())
		exit(-1);
	if(!DisabledSystem::GetDisabled(SYS_REFERENCE) && !ReferenceManager::GetInstance()->Initialize())
		exit(-1);
	if(!DisabledSystem::GetDisabled(SYS_REWARD) && !RewardManager::GetInstance()->Initialize())
		exit(-1);
	Log::setprogname("gdeliveryd");
	{
		manager->freeaid=atoi(conf->find(manager->Identification(), "freeaid").c_str());
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		manager->zoneid=atoi(conf->find(manager->Identification(), "zoneid").c_str());
		manager->aid=atoi(conf->find(manager->Identification(), "aid").c_str());
		manager->district_id=atoi(conf->find(manager->Identification(), "district_id").c_str());
		std::cerr<<"District id is "<<manager->district_id << std::endl;
		manager->freecreatime=atoi(conf->find(manager->Identification(), "freecreatime").c_str());
		std::string algo_str = conf->find(manager->Identification(), "challenge_algo");
		if (!algo_str.length()) 
			manager->challenge_algo=0;
		else
			manager->challenge_algo=atoi(algo_str.c_str());
		if( strcmp(conf->find(manager->Identification(), "token_login_support").c_str(),"true")==0 )
			manager->token_login_support = true;

		std::string max_name_len = conf->find(manager->Identification(), "max_name_len");
		if(!max_name_len.empty())
			manager->max_name_len = atoi(max_name_len.c_str());

		int max_player_num=atoi(conf->find(manager->Identification(), "max_player_num").c_str());
		max_player_num=max_player_num<MAX_PLAYER_NUM_DEFAULT ? MAX_PLAYER_NUM_DEFAULT:max_player_num;
		UserContainer::GetInstance().SetPlayerLimit( max_player_num,max_player_num );
		DEBUG_PRINT("gdeliveryd::Max player allowed is %d\n",UserContainer::GetInstance().GetPlayerLimit());
		
		//读取内网ip列表
		std::string ip_list_str = conf->find(manager->Identification(), "lan_ip_list");
		UserContainer::GetInstance().InitLanIpList( ip_list_str );

		int debug = atoi(conf->find(manager->Identification(), "debugmode").c_str());
		if(debug)                       
			manager->SetDebugmode(debug);  
		if (0==manager->zoneid) 
		{
			std::cerr<<"Invalid zone id(0). Check .conf file."<<std::endl;
			exit(-1);
		}
		if ( manager->IsFreeZone() )
			manager->serverAttr.SetFreeZone(1);
		std::cout<<"zoneid="<<(int)manager->zoneid<<"  aid="<<(int)manager->aid<<std::endl;
		Protocol::Server(manager);

		if(!DisabledSystem::GetDisabled(SYS_GAMETALK)) {
			GameTalkManager::GetInstance()->OnStartUp((int)manager->aid, (unsigned char)manager->zoneid);
		}
	}
	
	if (!is_central) 
	{
		GAuthClient *manager = GAuthClient::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		std::string au=conf->find(manager->Identification(),"shared_key");
		manager->shared_key=Octets(au.c_str(),au.size());
		if( strcmp(conf->find(manager->Identification(), "au_cert").c_str(),"false")==0 )
			manager->au_cert = false;
		Protocol::Client(manager);
	}
	{
		GProviderServer *manager = GProviderServer::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		manager->SetProviderServerID(atoi(conf->find(manager->Identification(), "id").c_str()));
		Protocol::Server(manager);
	}	
	{
		GAntiCheatClient *manager = GAntiCheatClient::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
	}
	if(!is_central)
	{
		UniqueNameClient* manager=UniqueNameClient::GetInstance();
		Protocol::Client(manager);
		Thread::HouseKeeper::AddTimerTask(new KeepAliveTask(30),30); 
	}
	if(!DisabledSystem::GetDisabled(SYS_WEBTRADE))
	{
		GWebTradeClient *manager = GWebTradeClient::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
	}
	if(!DisabledSystem::GetDisabled(SYS_GAMETALK))
	{
		GameTalkClient *manager = GameTalkClient::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
	}
	if(!DisabledSystem::GetDisabled(SYS_SNS))
	{
		SNSClient *manager = SNSClient::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
	}


	int pvp = atoi(conf->find(GDeliveryServer::GetInstance()->Identification(), "pvp").c_str());
	if ( pvp ) 
	{
		GDeliveryServer::GetInstance()->serverAttr.SetPVP(1);
		GameAttrMap::Put(Privilege::PRV_PVP, GameAttrMap::_ATTR_ENABLE); 
	}
	if (!DisabledSystem::GetDisabled(SYS_BATTLE))
	{
		if( !BattleManager::GetInstance()->Initialize())
		{
			Log::log(LOG_ERR,"Fatal: Initialize domain data failed.");
   			exit(EXIT_FAILURE);
		}
		else
			GDeliveryServer::GetInstance()->serverAttr.SetBattle(1);
	}
		
	if (!DisabledSystem::GetDisabled(SYS_AUCTION) && !AuctionMarket::GetInstance().Initialize())
	{
		Log::log(LOG_ERR,"Fatal: Initialize auction category data failed.");
   		exit(EXIT_FAILURE);
	}
	if (!DisabledSystem::GetDisabled(SYS_WEBTRADE) && !WebTradeMarket::GetInstance().Initialize())
	{
		Log::log(LOG_ERR,"Fatal: Initialize webtrade market failed.");
   		exit(EXIT_FAILURE);
	}
	if (!DisabledSystem::GetDisabled(SYS_SYSAUCTION) && !SysAuctionManager::GetInstance().Initialize())
	{
		Log::log(LOG_ERR,"Fatal: Initialize sysauction failed.");
   		exit(EXIT_FAILURE);
	}
	if(!DisabledSystem::GetDisabled(SYS_FACTIONFORTRESS) && !FactionFortressMan::GetInstance().Initialize())
	{
		Log::log(LOG_ERR,"Fatal: Initialize factionfortress failed.");	
		exit(EXIT_FAILURE);
	}
	if(!ForceManager::GetInstance()->Initialize())
	{
		Log::log(LOG_ERR,"Fatal: Initialize force failed.");
		exit(EXIT_FAILURE);
	}
	if(!DisabledSystem::GetDisabled(SYS_KINGELECTION) && !KingElection::GetInstance().Initialize())
	{
		Log::log(LOG_ERR,"Fatal: Initialize kingelection failed.");
		exit(EXIT_FAILURE);
	}
	if(!DisabledSystem::GetDisabled(SYS_PLAYERSHOP) && !PShopMarket::GetInstance().Initialize())
	{
		Log::log(LOG_ERR,"Fatal: Initialize pshop market failed.");
		exit(EXIT_FAILURE);
	}
	if(!DisabledSystem::GetDisabled(SYS_PLAYERPROFILE) && !PlayerProfileMan::GetInstance()->Initialize())
	{
		Log::log(LOG_ERR,"Fatal: Initialize playerprofile failed.");
		exit(EXIT_FAILURE);
	}
	if(!DisabledSystem::GetDisabled(SYS_AUTOTEAM) && !AutoTeamMan::GetInstance()->Initialize())
	{
		Log::log(LOG_ERR,"Fatal: Initialize playerprofile failed.");
		exit(EXIT_FAILURE);
	}
	if(!DisabledSystem::GetDisabled(SYS_UNIQUEDATAMAN) && !UniqueDataServer::GetInstance()->Initialize())
	{
		Log::log(LOG_ERR,"Fatal: Initialize uniquedataman failed.");
		exit(EXIT_FAILURE);
	}
    if (!DisabledSystem::GetDisabled(SYS_MAPPASSWORD) && !Passwd::GetInstance().Initialize())
    {
        Log::log(LOG_ERR, "Fatal: Initialize mappassword failed.");
        exit(EXIT_FAILURE);
    }
	if(!DisabledSystem::GetDisabled(SYS_TANKBATTLE) && !TankBattleManager::GetInstance()->Initialize())
	{
		Log::log(LOG_ERR,"Fatal: Initialize tankbattlemanager failed.");
		exit(EXIT_FAILURE);
	}
    if(!DisabledSystem::GetDisabled(SYS_FACTIONRESOURCEBATTLE) && !FactionResourceBattleMan::GetInstance()->Initialize())
    {
        Log::log(LOG_ERR,"Fatal: Initialize factionresourcebattleman failed.");
        exit(EXIT_FAILURE);
    }

	FriendextinfoManager::GetInstance()->Initialize(!DisabledSystem::GetDisabled(SYS_RECALLOLDPLAYER));
	
	if (is_central) {
		LOG_TRACE("Central Delivery Server start to listen...");
		CentralDeliveryServer* cds = CentralDeliveryServer::GetInstance();
		cds->SetAccumulate(atoi(conf->find(cds->Identification(), "accumulate").c_str()));
		Protocol::Server(cds);

		//读取允许连接到跨服中央服务器的zone列表
		std::string zone_list_str = conf->find(cds->Identification(), "accepted_zone_list");
		int cnt = cds->InitAcceptedZoneList( zone_list_str );
		if(cnt < 2 || cnt > 4) {
			Log::log(LOG_ERR,"Fatal: Initialize central delivery failed. invalid accepted zoned count. count=%d\n", cnt);	
			exit(EXIT_FAILURE);
		}
		
		Thread::Pool::AddTask(new LoadExchangeTask(15));
		Thread::HouseKeeper::AddTimerTask(new CrsSvrCheckTimer(30), 30);
	} 
	else if(CheckIPAddress(conf->find(CentralDeliveryClient::GetInstance()->Identification(), "address").c_str()))
	{
		LOG_TRACE("Central Delivery Client start to connect...");
		CentralDeliveryClient* cdc = CentralDeliveryClient::GetInstance();
		Protocol::Client(cdc);
		Timer::Attach(cdc);
		cdc->SetAccumulate(atoi(conf->find(cdc->Identification(), "accumulate").c_str()));
	}
	else
	{
		LOG_TRACE("Server without connected to any central server");
	}
	
	//gamedbclinet需要读取CentralDelivery允许连接的zone_list，以便向gamedbserver发AnnounceCentralDelivery协议看双方允许的zone_list是否匹配，所以其初始化要放在CentralDelivery后面
	{
		GameDBClient *manager = GameDBClient::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
	}
	//factionclinet同样要发AnnounceCentralDelivery协议，但目前用不到允许连接的zone_list，不过为了统一，初始化也放在CentralDelivery后面
	{
		GFactionClient *manager = GFactionClient::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
	}
	
	//由于跨服的国战系统，需要读取CentralDelivery允许连接的zone_list，所以国战系统的初始化必须放在CentralDelivery初始化后面
	bool arrange_country_by_zoneid = false;
	if(is_central) arrange_country_by_zoneid = true;
	if(!DisabledSystem::GetDisabled(SYS_COUNTRYBATTLE) && !CountryBattleMan::GetInstance()->Initialize(arrange_country_by_zoneid))
	{
		Log::log(LOG_ERR,"Fatal: Initialize domain2 data failed.");
   		exit(EXIT_FAILURE);
	}
	
	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	
	int interval=atoi( conf->find("Intervals","account_interval").c_str() );
	interval=interval<600 ? 600 : interval;
	//interval=interval<60 ? 60 : interval;
	Thread::HouseKeeper::AddTimerTask(new RemoveForbidden(),MAX_REMOVETIME/2);

	interval=atoi( conf->find("Intervals","delrole_interval").c_str() );
	interval=interval<60 ? 60 : interval;
	Thread::HouseKeeper::AddTimerTask(DeleteRoleTask::GetInstance(interval/2),interval/2/* delay */);
	
	interval=atoi( conf->find("Intervals","checkforbidlogin_interval").c_str() );
	interval=interval<60 ? 60 : interval;
	Thread::HouseKeeper::AddTimerTask(CheckTimer::GetInstance(interval),interval/* delay */);
	Thread::HouseKeeper::AddTimerTask(new StatusAnnouncer(20),20);

	BillingAgent::Instance().Initialize();

	Thread::Pool::Run();
	return 0;
}

