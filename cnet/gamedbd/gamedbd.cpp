#include "gamedbserver.hpp"
#include "conf.h"
#include "log.h"
#include "thread.h"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>

#include "gamedbmanager.h"
#include "storage.h"
#include "storagetool.h"

#include "clsconfig.h"
#include "accessdb.h"
#include "selldataimage.h"
#include "transman.h"
#include "localmacro.h"
// Temp
#include "gterritorydetail"
#include "gterritorystore"

#include "forbidpolicy.h"
#include "xmlversion.h"
#include "friendextgain.h"

using namespace GNET;

static char conf_filename[256];
class DbPolicy : public Thread::Pool::Policy
{
public:
	virtual void OnQuit( )
	{
		StorageEnv::checkpoint( );
		StorageEnv::Close();
	}
};

static DbPolicy	s_policy;

// clsconfig db end
///////////////////////////////////////////////////////////////////////////////

void printhelp( const char * cmd )
{
	std::cerr << "Gamedbd version " << XMLVERSION << std::endl << std::endl;
	std::cerr << "Usage: " << cmd << " conf-file" << std::endl
			<< "\t[ importclsconfig | exportclsconfig | clearclsconfig | clearclswaypoint" << std::endl
			<< "\t| importrolelist rolelistfilenamelist| exportrolelist roleidfilename rolelistfilename" << std::endl
			<< "\t| printlogicuid | printunamerole zoneid | printunamefaction zoneid" << std::endl
			<< "\t| gennameidx | exportunique zoneid" << std::endl
			<< "\t| query roleid | exportrole roleid | importrole roleidfile | merge dbdatapath | mergewdb dbdatapath srczoneid" << std::endl
			<< "\t| listrole | listrolebrief | listuserbrief | listfaction | listfactionuser | listfactionrelation | listroleinventory" << std::endl
			<< "\t| listcity | listshoplog | listsyslog" << std::endl
			<< "\t| updateroles | convertdb | repairdb" << std::endl
			<< "\t| deletewaitdel | calcwaitdel" << std::endl
			<< std::endl
			<< "\t| tablestat | tablestatraw | findmaxsize dumpfilename" << std::endl
			<< "\t| read tablename id" << std::endl
			<< "\t| rewritetable fromname toname | rewritedb" << std::endl
			<< "\t| listid tablename | rewritetable roleidfile fromname toname" << std::endl
			<< "\t| compressdb | decompressdb" << std::endl
			<< "\t| walkalltable | exportgametalk outputdir" << std::endl
			<< "\t| exportuserstore roleid | exportprofittime | abstractroles dbdatapath zoneid | delzoneplayers zoneid" << std::endl
			<< "\t| setdbcrosstype type | listrolecrossinfo roleid | resetrolecrossinfo roleid remote_roleid data_timestamp cross_timestamp src_zoneid" << std::endl
			<< "\t| syncplayername | returncash useridfile | importrecalluser useridfile | querywebtrade sn | queryglobalcontrol" << std::endl
            << "\t| getsignindata month | setclspvpflag flag | getrolelogindata year logicid | getuserlogindata year logicid" << std::endl
            << "\t| listfintask | listmnfactioninfo | listmnfactionapplyinfo | listmndomaininfo | clearmnfactionid | clearmnfacioncredit" << std::endl
            << "\t| importmnfactioninfo mnfactioninfofile | importmnfactionapplyinfo mnfactionapplyinfofile | importmndomaininfo mndomaininfofile" << std::endl
            << "\t| setmnfactionstate state_num | listsolochallengerank | clearsolochallengerank roleid zoneid ]" << std::endl;
}

int main(int argc, char *argv[])
{
	if (argc < 2 || access(argv[1], R_OK) == -1 )
	{
		printf("Compiled By "__USER__ ", "__DATE__ " "__TIME__ "\n");
		printhelp(argv[0]);
		exit(-1);
	}

	setlinebuf(stdout);
	Conf *conf = Conf::GetInstance(argv[1]);
	strcpy(conf_filename,argv[1]);
	Log::setprogname("gamedbd");

	if(!StorageEnv::Open()){
		Log::log(LOG_ERR,"Initialize storage environment failed.\n");
		exit(-1);
	}

	if( argc == 3 && 0 == strcmp(argv[2],"importclsconfig") )
	{
		ClearClsConfig();
		ImportClsConfig();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"exportclsconfig") )
	{
		ExportClsConfig();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"clearclsconfig") )
	{
		ClearClsConfig();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"clearclswaypoint") )
	{
		ClearClsWayPoint();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"printlogicuid") )
	{
		PrintLogicuid( );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"printunamerole") )
	{
		PrintUnamerole( atoi(argv[3]) );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"printunamefaction") )
	{
		PrintUnamefaction( atoi(argv[3]) );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"gennameidx") )
	{
		GenNameIdx();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"exportunique") )
	{
		ExportUnique( atoi(argv[3]) );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"query") )
	{
		QueryRole( atoi(argv[3]) );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"exportrole") )
	{
		ExportRole( atoi(argv[3]) );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( (argc == 4 || argc == 5) && 0 == strcmp(argv[2],"clearrole") )
	{
		char * tablename = NULL;
		if(argc == 5) tablename = argv[4];
		ClearRole( atoi(argv[3]),tablename );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"exportuser") )
	{
		ExportUser( atoi(argv[3]) );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"merge") )
	{
		MergeDBAll( argv[3] );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( (argc == 5) && 0 == strcmp(argv[2],"mergewdb") )
	{
		int srczoneid = -1;
		srczoneid = atoi(argv[4]);
		CMGDB::MergeDBAll( argv[3], srczoneid );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"listrole") )
	{
		ListRole();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( (argc == 3 || argc == 4) && 0 == strcmp(argv[2],"listrolebrief") )
	{
		std::string login_time;
		if (argc == 4)
			login_time = std::string(argv[3]);
		ListRoleBrief(login_time);
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
 	else if( argc == 3 && 0 == strcmp(argv[2],"listuserbrief") )
	{
		ListUserBrief();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"listfaction") )
	{
		ListFaction();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"listfactionuser") )
	{
		ListFactionUser();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}	
	else if( argc == 3 && 0 == strcmp(argv[2],"listfactionrelation") )
	{
		ListFactionRelation();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"listcity") )
	{
		ListCity();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"listroleinventory") )
	{
		ListRoleInventory();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"listshoplog") )
	{
		ListShopLog();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"updateroles") )
	{
		UpdateRoles();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"convertdb") )
	{
		ConvertDB();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"repairdb") )
	{
		RepairDB();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"tablestat") )
	{
		TableStat();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"tablestatraw") )
	{
		TableStatRaw();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"findmaxsize") )
	{
		FindMaxsizeValue(argv[3]);
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 5 && 0 == strcmp(argv[2],"read") )
	{
		ReadTable( argv[3], atoi(argv[4]) );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 5 && 0 == strcmp(argv[2],"rewritetable") )
	{
		RewriteTable( argv[3], argv[4] );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"rewritedb") )
	{
		RewriteDB();
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"listid") )
	{
		ListId( argv[3] );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 6 && 0 == strcmp(argv[2],"rewritetable") )
	{
		RewriteTable( argv[3], argv[4], argv[5] );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"compressdb") )
	{
		CompressDB();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"listsyslog") )
	{
		ListSysLog();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"decompressdb") )
	{
		DecompressDB();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 5 && 0 == strcmp(argv[2], "exportrolelist"))
	{
		ExportRoleList(argv[3], argv[4]);
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if ( argc >= 4 && 0 == strcmp(argv[2], "importrolelist"))
	{
		ImportRoleList((const char **)&argv[3], argc-3);
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"walkalltable") )
	{
		WalkAllTable();		
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2], "exportgametalk"))
	{
		ExportGameTalkData(argv[3]);
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"exportuserstore") )
	{
		ExportUserStore( atoi(argv[3]) );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"exportprofittime") )
	{
		ExportProfitTime();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 5 && 0 == strcmp(argv[2],"abstractroles") )
	{
		AbstractPlayers( argv[3], atoi(argv[4]) );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"delzoneplayers") )
	{
		DelCentralZonePlayers( atoi(argv[3]) );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"setdbcrosstype") )
	{
		SetDBCrossType( (bool)atoi(argv[3]) );
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"listrolecrossinfo") )
	{
		ListRoleCrossInfo(atoi(argv[3]));
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 8 && 0 == strcmp(argv[2],"resetrolecrossinfo") )
	{
		ResetRoleCrossInfo(atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]));
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"syncplayername") )
	{
		SyncPlayerName();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"returncash") )
	{
		ReturnCash(argv[3]);
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"importrecalluser") )
	{
		ImportRecallUser(argv[3]);
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"querywebtrade") )
	{
		QueryWebTrade(int64_t(atoll(argv[3])));
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"queryglobalcontrol") )
	{
		QueryGlobalControl();
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return 0;
	}
    else if ((argc == 4) && (0 == strcmp(argv[2], "importrole")))
    {
        ImportRole(argv[3]);
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
	else if( argc == 4 && 0 == strcmp(argv[2],"deletewaitdel"))
	{
		bool access_continue = true;
		do
		{
			access_continue = DeleteWaitdel(atoi(argv[3]) > 0);
        	StorageEnv::checkpoint();
			StorageEnv::Close();
			StorageEnv::Open();
		}while(access_continue);
        StorageEnv::Close();
		return 0;
	}
	else if ((argc == 5) && (0 == strcmp(argv[2],"calcwaitdel")))
	{
		CalcWaitdel(atoi(argv[3]),atoi(argv[4]));	
        StorageEnv::checkpoint();
        StorageEnv::Close();
		return 0;
	}
    else if ((argc == 4) && (0 == strcmp(argv[2], "getsignindata")))
    {
        GetSigninData(atoi(argv[3]));
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
    else if ((argc == 4) && (0 == strcmp(argv[2], "setclspvpflag")))
    {
        SetClsPVPFlag(atoi(argv[3]));
        ExportClsConfig();
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
    else if ((argc == 5) && (0 == strcmp(argv[2], "getrolelogindata")))
    {
        GetRoleLoginData(atoi(argv[3]), atoi(argv[4]));
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
    else if ((argc == 5) && (0 == strcmp(argv[2], "getuserlogindata")))
    {
        GetUserLoginData(atoi(argv[3]), atoi(argv[4]));
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
	else if ((argc >= 3) && (0 == strcmp(argv[2], "listfintask")))
	{
       	int count = (argc == 4) ? atoi(argv[3]) : 2040;
		ListFinTaskCount(count); 
		StorageEnv::checkpoint();
        StorageEnv::Close();
		return 0;
	}
    else if ((argc == 3) && (0 == strcmp(argv[2], "listmnfactioninfo")))
    {
        ListMNFactionInfo();
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
    else if ((argc == 3) && (0 == strcmp(argv[2], "listmnfactionapplyinfo")))
    {
        ListMNFactionApplyInfo();
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
    else if ((argc == 3) && (0 == strcmp(argv[2], "listmndomaininfo")))
    {
        ListMNDomainInfo();
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
    else if ((argc == 3) && (0 == strcmp(argv[2], "clearmnfactionid")))
    {
        ClearMNFactionId();
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
    else if ((argc == 4) && (0 == strcmp(argv[2], "importmnfactioninfo")))
    {
        ImportMNFactionInfo(argv[3]);
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
    else if ((argc == 4) && (0 == strcmp(argv[2], "importmnfactionapplyinfo")))
    {
        ImportMNFactionApplyInfo(argv[3]);
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
    else if ((argc == 4) && (0 == strcmp(argv[2], "importmndomaininfo")))
    {
        ImportMNDomainInfo(argv[3]);
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
    else if ((argc == 4) && (0 == strcmp(argv[2], "setmnfactionstate")))
	{
		SetMNFactionState(atoi(argv[3]));
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
	}
    else if ((argc == 3) && (0 == strcmp(argv[2], "clearmnfacioncredit")))
	{
		ClearMNFactionCredit();
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
	}
    else if ((argc == 3) && (0 == strcmp(argv[2], "listsolochallengerank")))
    {
        ListSoloChallengeRank();
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
    else if ((argc == 5) && (0 == strcmp(argv[2], "clearsolochallengerank")))
    {
        ClearSoloChallengeRank(atoi(argv[3]), atoi(argv[4]));
        StorageEnv::checkpoint();
        StorageEnv::Close();
        return 0;
    }
	else if( argc >= 3 )
	{
		printhelp(argv[0]);
		StorageEnv::checkpoint();
		StorageEnv::Close();
		return -1;
	}
	if( 0 == atol(conf->find("gamedbd","noimportclsconfig").c_str()) )
	{
		Log::log( LOG_INFO, "Begin import clsconfig ..." );
		ImportClsConfig();
	}

	int zone_name_max_len = atoi(conf->find("gamedbd", "zone_name_max_len").c_str());
	if(zone_name_max_len <= 0 || zone_name_max_len > ZONE_NAME_MAX_LEN_UPLIMIT) zone_name_max_len = ZONE_NAME_MAX_LEN_DEFAULT;
	GameDBManager::GetInstance()->SetZoneNameMaxLen(zone_name_max_len);
	if(GameDBManager::GetInstance()->LoadZoneNameConfig() != 0)
	{
		Log::log( LOG_ERR, "Init GameDB, error initialize ZoneNameMap serverlist.sev" );
		exit(EXIT_FAILURE);
	}

	if (!GameDBManager::GetInstance()->InitGameDB())
	{
		Log::log( LOG_ERR, "Init GameDB, error initialize GameDB." );
		exit(EXIT_FAILURE);
	}
	else
	{
		Log::log( LOG_INFO, "Init GameDB successfully." );
	}
	{
		GameDBServer *manager = GameDBServer::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		std::string chargeset = conf->find(manager->Identification(), "adduser_on_charge");
		if(chargeset.size())
			GameDBManager::GetInstance()->SetAddUserOnChange(atoi(chargeset.c_str()));
		Protocol::Server(manager);
	}

	ForbidPolicy::GetInstance()->LoadPolicy("forbidpolicy.conf");

	Thread::Pool::AddTask(new CrsLogicuidSeeker());
	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	pthread_t	th,th1;
	pthread_create( &th, NULL, &StorageEnv::BackupThread, NULL );
	pthread_create( &th1, NULL, &FriendExtGainManager::UpdateLoginTime, NULL);
	Thread::Pool::Run( &s_policy );
	return 0;
}

