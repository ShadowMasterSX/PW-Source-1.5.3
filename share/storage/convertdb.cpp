#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <iostream>

#include "conf.h"
#include "log.h"

#if defined USE_BDB and defined USE_WDB

#include "storage.h"
#include "storagewdb.h"

namespace GNET
{

class ToWDBQuery : public BDB::StorageEnv::IQuery
{
public:
	std::string dbname;
	bool Update( BDB::StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		static int count = 0;
		if( ! (count++ % 100000) )
			printf( "\tconverting database records counter %d...\n", count );

		int roleid;

		try
		{
			WDB::StorageEnv::Storage * pstorage = WDB::StorageEnv::GetStorage(dbname.c_str());
			WDB::StorageEnv::AtomTransaction wtxn;
			try
			{
				pstorage->insert( key, value, wtxn );
			}
			catch ( WDB::DbException e ) { throw; }
			catch ( ... )
			{
				WDB::DbException ee( WDB::WDB_OLD_VERSION );
				wtxn.abort( ee );
				throw ee;
			}
		}
		catch ( WDB::DbException e )
		{
			Log::log( LOG_ERR, "ToWDBQuery, roleid=%d, what=%s\n", roleid, e.what() );
		}

		if( ! (count % 1000000) )	WDB::StorageEnv::checkpoint();
		return true;
	}
};

void ToWDB( const char * dbname )
{
	printf( "\nconvert bdb to wdb %s:\n", dbname );

	std::string data_dir = BDB::StorageEnv::get_datadir();

	ToWDBQuery q;
	q.dbname = dbname;
	try
	{
		BDB::StorageEnv::Storage * pstorage = BDB::StorageEnv::GetStorage( dbname );
		BDB::StorageEnv::AtomTransaction	txn;
		try
		{
			BDB::StorageEnv::Storage::Cursor cursor = pstorage->cursor( txn );
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "ToWDB, error when walk, what=%s\n", e.what() );
	}

	WDB::StorageEnv::checkpoint();
}

void ToWDB( )
{
	printf( "\nconvert bdb format to wdb format\n" );

	BDB::StorageEnv::Open();
	WDB::StorageEnv::Open();

	std::string data_dir = BDB::StorageEnv::get_datadir();

	struct dirent **namelist;
	int n = scandir(data_dir.c_str(), &namelist, 0, alphasort);
	struct stat filestat;
	char indir[1024];

	if (n < 0)
		perror("scandir");
	else
	{
		while(n--)
		{
			sprintf(indir,"%s/%s",data_dir.c_str(),namelist[n]->d_name);
			stat(indir,&filestat);
			if(!S_ISDIR(filestat.st_mode) )
				ToWDB( namelist[n]->d_name );

			free(namelist[n]);
		}
		free(namelist);
	}

	WDB::StorageEnv::checkpoint();
	WDB::StorageEnv::removeoldlogs();
	WDB::StorageEnv::Close();
	BDB::StorageEnv::checkpoint();
	BDB::StorageEnv::Close();
}

class ToBDBQuery : public WDB::StorageEnv::IQuery
{
public:
	std::string dbname;
	bool Update( WDB::StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		static int count = 0;
		if( ! (count++ % 100000) )
			printf( "\tconverting database records counter %d...\n", count );

		int roleid;

		try
		{
			BDB::StorageEnv::Storage * pstorage = BDB::StorageEnv::GetStorage(dbname.c_str());
			BDB::StorageEnv::AtomTransaction btxn;
			try
			{
				pstorage->insert( key, value, btxn );
			}
			catch ( DbException e ) { throw; }
			catch ( ... )
			{
				DbException ee( DB_OLD_VERSION );
				btxn.abort( ee );
				throw ee;
			}
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "ToBDBQuery, roleid=%d, what=%s\n", roleid, e.what() );
		}

		if( ! (count % 1000000) )	BDB::StorageEnv::checkpoint();
		return true;
	}
};

void ToBDB( const char * dbname )
{
	printf( "\nconvert wdb to bdb %s:\n", dbname );

	std::string data_dir = WDB::StorageEnv::get_datadir();

	ToBDBQuery q;
	q.dbname = dbname;
	try
	{
		WDB::StorageEnv::Storage * pstorage = WDB::StorageEnv::GetStorage( dbname );
		WDB::StorageEnv::AtomTransaction	txn;
		try
		{
			WDB::StorageEnv::Storage::Cursor cursor = pstorage->cursor( txn );
			cursor.walk( q );
		}
		catch ( WDB::DbException e ) { throw; }
		catch ( ... )
		{
			WDB::DbException ee( WDB::WDB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
	}
	catch ( WDB::DbException e )
	{
		Log::log( LOG_ERR, "ToBDB, error when walk, what=%s\n", e.what() );
	}

	BDB::StorageEnv::checkpoint();
}

void ToBDB( )
{
	printf( "\nconvert wdb format to bdb format\n" );

	WDB::StorageEnv::Open();
	BDB::StorageEnv::Open();
	BDB::StorageEnv::checkpoint();

	std::string data_dir = WDB::StorageEnv::get_datadir();

	struct dirent **namelist;
	int n = scandir(data_dir.c_str(), &namelist, 0, alphasort);
	struct stat filestat;
	char indir[1024];

	if (n < 0)
		perror("scandir");
	else
	{
		while(n--)
		{
			sprintf(indir,"%s/%s",data_dir.c_str(),namelist[n]->d_name);
			stat(indir,&filestat);
			if(!S_ISDIR(filestat.st_mode) )
				ToBDB( namelist[n]->d_name );

			free(namelist[n]);
		}
		free(namelist);
	}

	BDB::StorageEnv::checkpoint();
	BDB::StorageEnv::removeoldlogs();
	BDB::StorageEnv::Close();
	WDB::StorageEnv::checkpoint();
	WDB::StorageEnv::Close();
}

};
#else
namespace GNET
{
void ToWDB( )
{
	std::cerr << "ERROR: recompile convertdb.cpp with -DUSE_DBD and -DUSE_WDB" << std::endl;
}
void ToBDB( )
{
	std::cerr << "ERROR: recompile convertdb.cpp with -DUSE_DBD and -DUSE_WDB" << std::endl;
}
};
#endif

using namespace GNET;

int main(int argc, char *argv[])
{
#if defined USE_BDB and defined USE_WDB
#else
	std::cerr << "ERROR: recompile convertdb.cpp with -DUSE_DBD and -DUSE_WDB" << std::endl;
	exit(-1);
#endif
	if (argc < 2 || access(argv[1], R_OK) == -1 )
	{
		std::cerr << "Usage: " << argv[0] << " conf-file [ towdb | tobdb ]" << std::endl;
		exit(-1);
	}

	Conf::GetInstance(argv[1]);
	Log::setprogname("bdbwdb_convert");

	if( argc == 3 && 0 == strcmp(argv[2],"towdb") )
	{
		ToWDB();
		return 0;
	}
	else if( argc == 3 && 0 == strcmp(argv[2],"tobdb") )
	{
		ToBDB();
		return 0;
	}
	else if( argc >= 3 )
	{
		return -1;
	}

	return 0;
}

