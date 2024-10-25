#include "uniquenameserver.hpp"
#include "conf.h"
#include "log.h"
#include "thread.h"
#include <iostream>
#include <unistd.h>

#include "storage.h"
#include "accessdb.h"
#include "xmlversion.h"

using namespace GNET;

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

void printhelp( const char * cmd )
{
	std::cerr << "Uniquenamed version " << XMLVERSION << std::endl << std::endl;
	std::cerr << "Usage: " << cmd << " configurefile " << std::endl
			<< "\t[ showinfo | setlogicuidnextid nextid | setfactionnextid nextid | setfamilynextid nextid" << std::endl
			<< "\t| queryuser userid | queryrolebyname name | queryfactionbyname name | queryfamilybyname name" << std::endl
			<< "\t| addlogicuid userid logicuid | addrole name zoneid roleid status" << std::endl
			<< "\t| addfaction name zoneid fid status | addfamily name zoneid fid status" << std::endl
			<< "\t| exportcsvlogicuid | exportcsvroleid | exportcsvrolename | exportcsvfaction | exportcsvfamily" << std::endl
			<< "\t| importcsvlogicuid filename | importcsvrole filename" << std::endl
			<< "\t| importcsvfaction filename | importcsvfamily filename" << std::endl
			<< "\t| merge dbdatapath ]" << std::endl;
}

int main(int argc, char *argv[])
{
	if (argc < 2 || access(argv[1], R_OK) == -1 )
	{
		printf("Compiled By "__USER__ ", "__DATE__ " "__TIME__ "\n");
		printhelp( argv[0] );
		exit(-1);
	}

	Conf *conf = Conf::GetInstance(argv[1]);
	Log::setprogname("uniquenamed");

	if(!StorageEnv::Open())
	{
		Log::log(LOG_ERR,"Initialize storage environment failed.\n");
		exit(-1);
	}

	if( argc == 3 && 0 == strcmp(argv[2],"showinfo") )
	{
		ShowInfo();
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"setlogicuidnextid") )
	{
		SetLogicuidNextid( atoi(argv[3]) );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"setfactionnextid") )
	{
		SetFactionNextid( atoi(argv[3]) );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"setfamilynextid") )
	{
		SetFamilyNextid( atoi(argv[3]) );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"queryuser") )
	{
		QueryUser( atoi(argv[3]) );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"queryrolebyname") )
	{
		QueryRoleByName( argv[3] );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"queryfactionbyname") )
	{
		QueryFactionByName( argv[3] );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"queryfamilybyname") )
	{
		QueryFamilyByName( argv[3] );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 5 && 0 == strcmp(argv[2],"addlogicuid") )
	{
		AddLogicuid( atoi(argv[3]), atoi(argv[4]) );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 7 && 0 == strcmp(argv[2],"addrole") )
	{
		AddRole( argv[3], atoi(argv[4]), atoi(argv[5]), atoi(argv[6]) );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 7 && 0 == strcmp(argv[2],"addfaction") )
	{
		AddFaction( argv[3], atoi(argv[4]), atoi(argv[5]), atoi(argv[6]) );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 7 && 0 == strcmp(argv[2],"addfamily") )
	{
		AddFamily( argv[3], atoi(argv[4]), atoi(argv[5]), atoi(argv[6]) );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"exportcsvlogicuid") )
	{
		ExportCsvLogicuid();
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"exportcsvroleid") )
	{
		ExportCsvRoleId();
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"exportcsvrolename") )
	{
		ExportCsvRoleName();
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"exportcsvfaction") )
	{
		ExportCsvFaction();
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"exportcsvfamily") )
	{
		ExportCsvFamily();
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"importcsvlogicuid") )
	{
		ImportCsvLogicuid( argv[3] );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"importcsvrole") )
	{
		ImportCsvRole( argv[3] );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"importcsvfaction") )
	{
		ImportCsvFaction( argv[3] );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"importcsvfamily") )
	{
		ImportCsvFamily( argv[3] );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc == 4 && 0 == strcmp(argv[2],"merge") )
	{
		MergeDBAll( argv[3] );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return 0;
	}
	else if( argc >= 3 )
	{
		printhelp( argv[0] );
		StorageEnv::checkpoint( );
		StorageEnv::Close();
		return -1;
	}

	{
		UniqueNameServer *manager = UniqueNameServer::GetInstance();
		std::string caseflag = conf->find(manager->Identification(),"case_insensitive");
		if(caseflag.size())
			manager->SetSensitivity(atoi(caseflag.c_str()));

		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Server(manager);
	}

	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	Thread::HouseKeeper::AddTimerTask(new LogicuidSeeker(), 5);
	pthread_t	th;
	pthread_create( &th, NULL, &StorageEnv::BackupThread, NULL );
	Thread::Pool::Run( &s_policy );
	return 0;
}

