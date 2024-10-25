#include <sys/types.h>
#include <dirent.h>

#include <set>
#include <map>
#include <vector>
#include <fstream>
#include <string>

#include "storagetool.h"
#include "log.h"

namespace GNET
{
bool EscapeCSVString( Octets & src, Octets & dest )
{
	dest.clear();
	dest.insert( dest.end(), "\"", 1 );
	for( size_t i=0; i<src.size(); i++ )
	{
		if( '\"' == *(((char*)src.begin()) + i) )
			dest.insert( dest.end(), "\"\"", 2 );
		else
			dest.insert( dest.end(), ((char*)src.begin())+i, 1 );
	}
	dest.insert( dest.end(), "\"", 1 );
	return true;
}

bool UnescapeCSVString( Octets & src, Octets & dest )
{
	dest.clear();
	for( size_t i=0; i<src.size(); i++ )
	{
		if( (0 == i||src.size()-1 == i) && '\"' == *(((char*)src.begin()) + i) )
			continue;

		if( '\"' == *(((char*)src.begin()) + i) )
		{
			dest.insert( dest.end(), "\"\"", 2 );
			if( i+1<src.size() && '\"' == *(((char*)src.begin()) + i + 1) )
				i ++;
		}
		else
			dest.insert( dest.end(), ((char*)src.begin())+i, 1 );
	}
	return true;
}

class TableStatQuery : public StorageEnv::IQuery
{
public:
	size_t maxsize;
	size_t count;
	long long sum;

	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		count ++;
		if( value.size() > maxsize )	maxsize = value.size();
		sum += value.size();
		return true;
	}
};

void TableStat( const char * dbname )
{
	std::string data_dir = StorageEnv::get_datadir();

	TableStatQuery q;
	q.maxsize = q.count = 0;
	q.sum = 0;
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
		Log::log( LOG_ERR, "TableStat, error when walk, what=%s\n", e.what() );
	}
	printf( "\t%s", dbname );
	int len = strlen(dbname);
	for( ; len<20; len++ )	printf( " " );
	len = printf( "%u", q.maxsize );
	for( ; len<20; len++ )	printf( " " );
	len = printf( "%u", q.count );
	for( ; len<20; len++ )	printf( " " );
	printf( "%lf\n", ((double)q.sum) / q.count );
}

void TableStat( )
{
	printf( "\nTable Statistics:\n" );
	printf( "\tTable Name          MaxValueSize        RecordCount         Mean\n" );

	std::string data_dir = StorageEnv::get_datadir();

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
			if(!S_ISDIR(filestat.st_mode) && 0 != strcmp(STORAGE_CONFIGDB,namelist[n]->d_name) )
				TableStat( namelist[n]->d_name );

			free(namelist[n]);
		}
		free(namelist);
	}
}

void TableStatRaw( const char * dbname )
{
	std::string data_dir = StorageEnv::get_datadir();

	TableStatQuery q;
	q.maxsize = q.count = 0;
	q.sum = 0;
	try
	{
		StorageEnv::Storage * pstorage = StorageEnv::GetStorage( dbname );
		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor = pstorage->cursor( txn );
			pstorage->SetCompressor( new StorageEnv::NullCoder(),new StorageEnv::NullCoder());
#if defined(USE_WDB) && ! defined(USE_BDB)
			cursor.walk_raw( q );
#else
			cursor.walk( q );
#endif
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
		Log::log( LOG_ERR, "TableStat, error when walk, what=%s\n", e.what() );
	}
	printf( "\t%s", dbname );
	int len = strlen(dbname);
	for( ; len<20; len++ )	printf( " " );
	len = printf( "%u", q.maxsize );
	for( ; len<20; len++ )	printf( " " );
	len = printf( "%u", q.count );
	for( ; len<20; len++ )	printf( " " );
	printf( "%lf\n", ((double)q.sum) / q.count );
}

void TableStatRaw( )
{
	printf( "\nTable Statistics:\n" );
	printf( "\tTable Name          MaxValueSize        RecordCount\n" );

	std::string data_dir = StorageEnv::get_datadir();

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
			if(!S_ISDIR(filestat.st_mode) && 0 != strcmp(STORAGE_CONFIGDB,namelist[n]->d_name) )
				TableStatRaw( namelist[n]->d_name );

			free(namelist[n]);
		}
		free(namelist);
	}
}

class FindMaxsizeValueQuery : public StorageEnv::IQuery
{
public:
	size_t rows;
	size_t maxsize;
	Octets mkey;
	Octets mvalue;
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		++rows;

		try
		{
			if( value.size() > maxsize )
			{
				maxsize = value.size();
				mkey = key;
				mvalue = value;
			}
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "FindMaxsizeValueQuery, %s\n", e.what() );
		}
		return true;
	}
};

void FindMaxsizeValue( const char *dumpfile )
{
	FILE *f = fopen(dumpfile, "w");
	if( f == NULL ) return;

	printf( "\nfind maxsize value of each table, dump to %s\n", dumpfile);

	std::string data_dir = StorageEnv::get_datadir();

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
			{
				const char *dbname = namelist[n]->d_name;
				//
				printf( "\nfind maxsize value of table %s:\n", dbname );

				FindMaxsizeValueQuery q;
				q.maxsize = 0;
				q.rows = 0;
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
					Log::log( LOG_ERR, "find maxsize value, error when walk, what=%s\n", e.what() );
				}
				printf( "\t\t rows is %d, maxsize is %d\n", q.rows, q.maxsize);
				fprintf(f, "table %s\n", dbname);
				fprintf(f, "\tcount %d\n", q.rows);
				fprintf(f, "\tmaxsize %d\n", q.maxsize);
				fprintf(f, "\tkey:\n");
				for(size_t j=0;j<q.mkey.size();++j)
					fprintf(f, "%02x", *((unsigned char*)(q.mkey.begin())+j));
				fprintf(f, "\n");
				fprintf(f, "\tvalue:\n");
				for(size_t j=0;j<q.mvalue.size();++j)
					fprintf(f, "%02x", *((unsigned char*)(q.mvalue.begin())+j));
				fprintf(f, "\n");
				fprintf(f, "\n");
				//
			}

			free(namelist[n]);
		}   
		free(namelist);
	}

	fclose(f);
}

void ReadTable( const char * tablename, int key )
{
	try
	{
		StorageEnv::Storage * pstorage = StorageEnv::GetStorage(tablename);
		StorageEnv::AtomTransaction txn;
		try
		{
			Octets value_raw, value;
			Marshal::OctetsStream	os_key;
			os_key << key;

#if defined(USE_WDB) && ! defined(USE_BDB)
			pstorage->SetCompressor( new StorageEnv::NullCoder(),new StorageEnv::NullCoder());
			if( pstorage->find( os_key, value_raw, txn ) )
			{
				printf("the raw data for key %d in table %s:\n", key, tablename );
				value_raw.dump();
			}
			else
			{
				printf("cannot find the raw data for key %d in table %s.\n", key, tablename );
			} 

			pstorage->SetCompressor( new StorageEnv::Compressor(),new StorageEnv::Uncompressor());
			if( pstorage->find( os_key, value, txn ) )
			{
				printf("the decompressed data for key %d in table %s:\n", key, tablename );
				value.dump();
			}
			else
			{
				printf("cannot find the decompressed data for key %d in table %s.\n", key, tablename );
			}
#else
			value_raw = pstorage->find_nocompress( os_key, txn );
			printf("the raw data for key %d in table %s:\n", key, tablename );
			value_raw.dump();

			if( pstorage->find( os_key, value, txn ) )
			{
				printf("the decompressed data for key %d in table %s:\n", key, tablename );
				value.dump();
			}
			else
			{
				printf("cannot find the decompressed data for key %d in table %s.\n", key, tablename );
			}
#endif
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
		Log::log( LOG_ERR, "ReadTable, tablename=%s, key=%d, what=%s\n", tablename, key, e.what() );
	}
}

class RewriteTableQuery : public StorageEnv::IQuery
{
public:
#if defined(USE_WDB) && ! defined(USE_BDB)
	DBStandalone * pto;
#else
	StorageEnv::Storage * pto;
#endif

	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		static int count = 0;
		if( ! (count++ % 100000) ) printf( "\tconverting database records counter %d...\n", count );

		try
		{
			try
			{
#if defined(USE_WDB) && ! defined(USE_BDB)
				Octets value_new = (new StorageEnv::Compressor())->Update( value );
				pto->put( key.begin(), key.size(), value_new.begin(), value_new.size() );
#else
				pto->insert( key, value, txn );
#endif
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
			Log::log( LOG_ERR, "RewriteTableQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void RewriteTable( const char * fromname, const char * toname )
{
	printf( "\nrewrite table from %s to %s.\n", fromname, toname );

#if defined(USE_WDB) && ! defined(USE_BDB)
	DBStandalone * pto = new DBStandalone( (StorageEnv::get_datadir()+"/"+toname).c_str(), 50000, 45000 );
#endif
	RewriteTableQuery q;
	try
	{
		StorageEnv::Storage * pfrom = StorageEnv::GetStorage( fromname );
#if defined(USE_WDB) && ! defined(USE_BDB)
#else
		StorageEnv::Storage * pto = StorageEnv::GetStorage( toname );
#endif
		StorageEnv::AtomTransaction	txn;
		q.pto = pto;
		try
		{
			StorageEnv::Storage::Cursor cursor = pfrom->cursor( txn );
#if defined(USE_WDB) && ! defined(USE_BDB)
			cursor.browse( q );
#else
			cursor.walk( q );
#endif
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
		Log::log( LOG_ERR, "RewriteTable, error when walk, what=%s\n", e.what() );
	}
#if defined(USE_WDB) && ! defined(USE_BDB)
	pto->checkpoint();
	delete pto;
#endif
}

void RewriteDB( )
{
	std::string data_dir = StorageEnv::get_datadir();

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
			if(!S_ISDIR(filestat.st_mode) && 0 != strcmp(STORAGE_CONFIGDB,namelist[n]->d_name) )
				RewriteTable( namelist[n]->d_name, "conv_temp" );

			StorageEnv::checkpoint( );
			StorageEnv::removeoldlogs( );
			StorageEnv::Close();
			if( 0 == access( (data_dir + "/conv_temp").c_str(), R_OK ) )	
				system( ("/bin/mv -f " + data_dir + "/conv_temp " + data_dir + "/" + namelist[n]->d_name).c_str() );
			StorageEnv::Open();

			free(namelist[n]);
		}
		free(namelist);
	}
}

class ListIdQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		Marshal::OctetsStream key_os, value_os;
		key_os = key;
		value_os = value;

		int roleid = 0;
		try
		{
			key_os >> roleid;
			if( 0 == roleid )
				return true;

			printf("%d\n",roleid);
		} catch ( Marshal::Exception & ) {
			Log::log( LOG_ERR, "ListIdQuery, error unmarshal, roleid=%d.", roleid );
			return true;
		}
		return true;
	}
};

void ListId( const char * tablename )
{
	ListIdQuery q;
	try
	{
		StorageEnv::Storage * pstorage = StorageEnv::GetStorage( tablename );
		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor = pstorage->cursor( txn );
#if defined(USE_WDB) && ! defined(USE_BDB)
			cursor.browse( q );
#else
			cursor.walk( q );
#endif
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
		Log::log( LOG_ERR, "ListId, error when walk, what=%s\n", e.what() );
	}
}

void RewriteTable( const char * keyidfile, const char * fromname, const char * toname )
{
	if( 0 != access(keyidfile,R_OK) )
	{
		fprintf( stderr, "keyidfile %s not found.\n", keyidfile );
		return;
	}

	std::ifstream	ifs( keyidfile );
	while( !ifs.eof() )
	{
		char	line[256];
		memset( line, 0, sizeof(line) );
		ifs.getline( line, sizeof(line) );
		line[sizeof(line)-1] = 0;
		if( !ifs.eof() && strlen(line) > 0 )
		{
#if defined(USE_WDB) && ! defined(USE_BDB)
			DBStandalone * pto = new DBStandalone( (StorageEnv::get_datadir()+"/"+toname).c_str(), 50000, 45000 );
#endif
			int roleid = atol( line );

			try
			{
				StorageEnv::Storage * pfrom = StorageEnv::GetStorage(fromname);
#if defined(USE_WDB) && ! defined(USE_BDB)
#else
				StorageEnv::Storage * pto   = StorageEnv::GetStorage(toname);
#endif
				StorageEnv::AtomTransaction txn;
				try
				{
					Octets value;
					Marshal::OctetsStream	key;
					key << roleid;

					if( pfrom->find( key, value, txn ) )
#if defined(USE_WDB) && ! defined(USE_BDB)
						pto->put( key.begin(), key.size(), value.begin(), value.size() );
#else
						pto->insert( key, value, txn );
#endif
					else
						printf( "roleid %d missing.\n", roleid );
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
				Log::log( LOG_ERR, "RewriteTable, roleid=%d, what=%s\n", roleid, e.what() );
			}
#if defined(USE_WDB) && ! defined(USE_BDB)
			pto->checkpoint();
			delete pto;
#endif
		}
	}
	ifs.close();
}

#if defined(USE_WDB) && ! defined(USE_BDB)
void CompressDB( )
{
	Log::log( LOG_ERR, "CompressDB, do not support WDB\n" );
}
void DecompressDB( )
{
	Log::log( LOG_ERR, "DecompressDB, do not support WDB\n" );
}
#else
class CompressDBQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		static int count = 0;
		if( ! (count++ % 100000) ) printf( "\tconverting database records counter %d...\n", count );

		try
		{
			StorageEnv::Storage * pconvtemp = StorageEnv::GetStorage("conv_temp");
			pconvtemp->SetCompressor( new StorageEnv::NullCoder(), new StorageEnv::NullCoder() );
			try
			{
				Octets value_com;
				GNET::Compress( value, value_com );
				pconvtemp->insert( key, value_com, txn );
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
			Log::log( LOG_ERR, "CompressDBQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void CompressDB( const char * dbname )
{
	printf( "\ncompress database %s:\n", dbname );

	std::string data_dir = StorageEnv::get_datadir();

	CompressDBQuery q;
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
		Log::log( LOG_ERR, "CompressDB, error when walk, what=%s\n", e.what() );
	}
	StorageEnv::checkpoint( );
	StorageEnv::removeoldlogs( );
	StorageEnv::Close();
	if( 0 == access( (data_dir + "/conv_temp").c_str(), R_OK ) )	
		system( ("/bin/mv -f " + data_dir + "/conv_temp " + data_dir + "/" + dbname).c_str() );
	StorageEnv::Open();
}

void CompressDB( )
{
	std::string data_dir = StorageEnv::get_datadir();

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
			if(!S_ISDIR(filestat.st_mode) && 0 != strcmp(STORAGE_CONFIGDB,namelist[n]->d_name) )
				CompressDB( namelist[n]->d_name );

			free(namelist[n]);
		}
		free(namelist);
	}

	StorageEnv::writecompressstatus( true );
}

class DecompressDBQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		static int count = 0;
		if( ! (count++ % 100000) ) printf( "\tconverting database records counter %d...\n", count );

		try
		{
			StorageEnv::Storage * pconvtemp = StorageEnv::GetStorage("conv_temp");
			pconvtemp->SetCompressor( new StorageEnv::NullCoder(), new StorageEnv::NullCoder() );
			try
			{
				pconvtemp->insert( key, value, txn );
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
			Log::log( LOG_ERR, "DecompressDBQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void DecompressDB( const char * dbname )
{
	printf( "\ndecompress database %s:\n", dbname );

	std::string data_dir = StorageEnv::get_datadir();

	DecompressDBQuery q;
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
		Log::log( LOG_ERR, "CompressDB, error when walk, what=%s\n", e.what() );
	}
	StorageEnv::checkpoint( );
	StorageEnv::removeoldlogs( );
	StorageEnv::Close();
	if( 0 == access( (data_dir + "/conv_temp").c_str(), R_OK ) )	
		system( ("/bin/mv -f " + data_dir + "/conv_temp " + data_dir + "/" + dbname).c_str() );
	StorageEnv::Open();
}

void DecompressDB( )
{
	std::string data_dir = StorageEnv::get_datadir();

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
			if(!S_ISDIR(filestat.st_mode) && 0 != strcmp(STORAGE_CONFIGDB,namelist[n]->d_name) )
				DecompressDB( namelist[n]->d_name );

			free(namelist[n]);
		}
		free(namelist);
	}

	StorageEnv::writecompressstatus( false );
}
#endif

}

