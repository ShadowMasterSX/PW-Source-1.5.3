
#include <vector>
#include <map>
#include <iostream>
#include <netinet/in.h>
#include <string>

#include "storage.h"
#include "conf.h"

using namespace std;
using namespace GNET;

class ConvertQuery : public StorageEnv::IQuery
{
	std::vector<std::string>	dbnames;
public:
	ConvertQuery(std::vector<std::string> & names) : dbnames(names) { }
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value )
	{
		for( size_t i=0; i<dbnames.size(); i++ )
		{
			StorageEnv::Storage * pstorage1 = StorageEnv::GetStorage( dbnames[i].c_str() );
			StorageEnv::Storage * pstorage2 = StorageEnv::GetStorage( (dbnames[i]+".bak").c_str() );

			Octets v;
			try
			{
				StorageEnv::AtomTransaction	txn;
				try
				{
					if( 0 == i )
						v = value;
					else
						v = pstorage1->find( const_cast<Octets&>(key), txn );
					pstorage2->insert( const_cast<Octets&>(key), v, txn );
				}
				catch ( DbException e )
				{
					throw;
				}
				catch ( ... )
				{
					DbException ee( DB_OLD_VERSION );
					txn.abort( ee );
					throw ee;
				}
			}
			catch ( DbException e )
			{
				printf( "Error when walk, what=%s\n", e.what() );
			}
		}
	
		return true;
	}
};

void ConvertDB( std::vector<std::string> & dbnames )
{
	if( dbnames.size() <= 0 )
		return;

	StorageEnv::Storage * pstorage = StorageEnv::GetStorage( dbnames[0].c_str() );

	Octets key;
	ConvertQuery q( dbnames );

	try
	{
		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor = pstorage->cursor( txn );
			cursor.walk( q );
		}
		catch ( DbException e )
		{
			throw;
		}
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
	}
	catch ( DbException e )
	{
		printf( "Error when walk, what=%s\n", e.what() );
	}

	StorageEnv::checkpoint();
}

int main( int argc, char **argv )
{
	if (argc < 3 && access(argv[1], R_OK) == -1)
	{
		std::cerr << "Usage: " << argv[0] << " configurefile dbname" << std::endl;
		exit(-1);
	}

	GNET::Conf::GetInstance(argv[1]);
	StorageEnv::Open();

	std::vector<std::string>	dbnames;
	for( int i=2; i<argc; i++ )
		dbnames.push_back( argv[i] );

	ConvertDB( dbnames );

	StorageEnv::Close();
	return (EXIT_SUCCESS);
}


