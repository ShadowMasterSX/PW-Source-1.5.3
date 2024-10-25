
#include <vector>
#include <map>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <mcheck.h>

#include "conf.h"
#include "statistic.h"
#include "benchmark.h"
#include "storage.h"
#include "dbbuffer.h"
#include "querybinder.h"

using namespace std;
using namespace GNET;

int insertmass( const char * dbname, std::vector<int> &intvec, Octets &value )
{
	int ret = 0;
	for( size_t i=0; i<intvec.size(); i++ )
	{
		Octets key(&intvec[i],sizeof(int));
		key.insert( key.end(), &intvec[i],sizeof(int));

		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage( dbname );
			StorageEnv::AtomTransaction	txn;
			try
			{
				pstorage->insert( key, value, txn );
				ret ++;
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
			printf( "Error when insert key:%d, what=%s\n", intvec[i], e.what() );
		}
	}
	return ret;
}

int findmass( const char * dbname, std::vector<int> &intvec )
{
	int ret = 0;
	Octets value;
	for( size_t i=0; i<intvec.size(); i++ )
	{
		Octets key(&intvec[i],sizeof(int));
		key.insert( key.end(), &intvec[i],sizeof(int));

		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage( dbname );
			StorageEnv::AtomTransaction	txn;
			try
			{
				value = pstorage->find( key, txn );
				ret ++;
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
			printf( "Unknown Error when find key:%d, what=%s\n", intvec[i], e.what() );
		}
	}
	return ret;
}

int delmass( const char * dbname, std::vector<int> &intvec )
{
	int ret = 0;
	for( size_t i=0; i<intvec.size(); i++ )
	{
		Octets key(&intvec[i],sizeof(int));
		key.insert( key.end(), &intvec[i],sizeof(int));

		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage( dbname );
			StorageEnv::AtomTransaction	txn;
			try
			{
				pstorage->del( key, txn );
				ret ++;
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
			printf( "Error when del key:%d, what=%s\n", intvec[i], e.what() );
		}
	}
	return ret;
}

class MyQuery : public StorageEnv::IQuery
{
public:
	int count;
	std::vector<std::pair<Octets,Octets> > result;
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		count ++;
		return true;
	}
};

int bulkquerytest( const char * dbname )
{
	Benchmark bmQuery;
	bmQuery.tickbegin();

	MyQuery q;
	q.count = 0;

	try
	{
		StorageEnv::Storage * pstorage = StorageEnv::GetStorage( dbname );
		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor = pstorage->cursor( txn );
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
		printf( "Error when walk, what=%s\n", e.what() );
	}

	printf( "walk count = %d\n", q.count );

	bmQuery.tickend();
	cout << "\nQuery Timing:\n";
	bmQuery.outputtickstat( cout );
	bmQuery.outputtickvalues( cout );

	return 0;
}

int main( int argc, char **argv )
{
	if (argc < 5 || access(argv[1], R_OK) == -1)
	{
		std::cerr << "Usage: " << argv[0] << " configurefile dbname insert|find|del count"<<std::endl;
		exit(-1);
	}

	GNET::Conf::GetInstance(argv[1]);
	srand(time(NULL));

	std::string dbname = "role";
	if( argc > 2 )
		dbname = argv[2];

	// open db
	StorageEnv::Open();

	{
		StorageEnv::Storage * pstorage = StorageEnv::GetStorage( dbname.c_str() );
		StorageEnv::AtomTransaction	txn;

		try
		{
			printf( "\nInitial Count %d.\n\n", pstorage->count() );
			pstorage->find( Octets("1",1), txn );
		}
		catch ( DbException ee )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
		}
	}

	// benchmark
	Benchmark bmFind, bmInsert, bmDel, bmAll;

	// insert and find
	int count = 0;
	if( argc > 3 )
		count = atol(argv[4]);
	std::vector<int> intvec;
	for ( int i=1; i<=count; i++ ) intvec.push_back(i);

	char buffer[4000];
	memset( buffer, 0, sizeof(buffer) );
	for( size_t j=0; j<sizeof(buffer); j++ )
		buffer[j] = 'A' + rand() % 26;
	Octets value(buffer,sizeof(buffer));

	int ret, span;
	bmAll.tickbegin();

	printf( "begin to do.\n" );
	if( 0 == strcmp( "insert", argv[3] ) )
	{
		std::random_shuffle( intvec.begin(), intvec.end());
		bmInsert.tickbegin();
		ret = insertmass( dbname.c_str(), intvec, value );
		span = bmInsert.tickend();
		printf( "%u for insert %d/%d\n", span, ret, intvec.size() );
	}
	if( 0 == strcmp( "find", argv[3] ) )
	{
		std::random_shuffle( intvec.begin(), intvec.end());
		bmFind.tickbegin();
		ret = findmass( dbname.c_str(), intvec );
		span = bmFind.tickend();
		printf( "%u for find %d/%d\n", span, ret, intvec.size() );
	}
	if( 0 == strcmp( "del", argv[3] ) )
	{
		std::random_shuffle( intvec.begin(), intvec.end());
		bmDel.tickbegin();
		ret = delmass( dbname.c_str(), intvec );
		span = bmDel.tickend();
		printf( "%u for del %d/%d\n", span, ret, intvec.size() );
	}
	if( 0 == strcmp( "walk", argv[3] ) )
	{
		bulkquerytest( dbname.c_str() );
	}

	printf( "checkpoint begin.\n" );
	StorageEnv::checkpoint();
	printf( "checkpoint end.\n" );
	StorageEnv::removeoldlogs();
	bmAll.tickend();

	// output statistic
	cout << "Insert Timing:";
	bmInsert.outputtickstat( cout );
	bmInsert.outputtickvalues( cout );
	cout << "Find Timing:";
	bmFind.outputtickstat( cout );
	bmFind.outputtickvalues( cout );
	cout << "Del Timing:";
	bmDel.outputtickstat( cout );
	bmDel.outputtickvalues( cout );
	cout << "All Timing:";
	bmAll.outputtickstat( cout );
	bmAll.outputtickvalues( cout );

	//cout << "\nusage:\n";
	//bmFind.outputusage( cout );

	//Statistic::enumerate( Statistic::enumdefault );

	StorageEnv::Close();

	return (EXIT_SUCCESS);
}


