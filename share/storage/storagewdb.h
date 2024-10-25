#ifndef __STORAGEWDB_H
#define __STORAGEWDB_H

#include "db.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <set>
#include <map>

#include "octets.h"
#include "conf.h"
#include "mutex.h"
#include "compress.h"
#include "reference.h"
#include "parsestring.h"
#include "log.h"

#define WDB_HOMEDIR_DEFAULT "./dbhome"
#define WDB_DATADIR_DEFAULT "./dbdata"
#define WDB_LOGDIR_DEFAULT  "./dblogs"

#ifndef STRICTNAMESPACE
using namespace GNET;
#endif

namespace WDB
{

enum {
	WDB_NOOVERWRITE = 1, WDB_OK, WDB_NOTFOUND,
	WDB_OVERWRITE, WDB_OLD_VERSION, WDB_LOCK_DEADLOCK,
	WDB_VERIFY_BAD, WDB_KEYSIZE_ZERO
};
#ifndef DB_NOOVERWRITE
#define	DB_NOOVERWRITE		WDB_NOOVERWRITE
#define DB_OK				WDB_OK
#define	DB_NOTFOUND			WDB_NOTFOUND
#define DB_OVERWRITE		WDB_OVERWRITE
#define	DB_OLD_VERSION		WDB_OLD_VERSION
#define	DB_LOCK_DEADLOCK	WDB_LOCK_DEADLOCK
#define	DB_VERIFY_BAD		WDB_VERIFY_BAD
#define	DB_KEYSIZE_ZERO		WDB_KEYSIZE_ZERO
#endif
class DbException
{
	int ex;
public:
	enum { WDB_OK = WDB::WDB_OK, WDB_NOTFOUND = WDB::WDB_NOTFOUND, WDB_OVERWRITE = WDB::WDB_OVERWRITE };
	DbException( int e ) : ex(e) { }
	int get_errno() { return ex; }
	const char * what()
	{
		switch( ex )
		{
			case WDB_OK:				return "OK";
			case WDB_NOTFOUND:		return "NOTFOUND";
			case WDB_OVERWRITE:		return "OVERWRITE";
			case WDB_NOOVERWRITE:	return "NOOVERWRITE";
			case WDB_OLD_VERSION:	return "OLD_VERSION";
			case WDB_LOCK_DEADLOCK:	return "LOCK_DEADLOCK";
			case WDB_VERIFY_BAD:		return "VERIFY_BAD";
			case WDB_KEYSIZE_ZERO:		return "KEYSIZE_ZERO";
			default:				return "UNKNOWN";
		}
	}
};

class StorageEnv
{
public:
	struct DataCoder
	{
		virtual ~DataCoder() { }
		virtual GNET::Octets Update (GNET::Octets os) = 0;
	};

	struct NullCoder : public DataCoder
	{
		GNET::Octets Update (GNET::Octets os) { return os; }
	};

	struct Compressor : public DataCoder
	{
		GNET::Octets Update (GNET::Octets os_src)
		{
			GNET::Octets os_com;
			GNET::Compress(os_src, os_com);
			return os_com;
		}
	};

	struct Uncompressor : public DataCoder
	{
		GNET::Octets Update (GNET::Octets os_com)
		{
			GNET::Octets os_src;
			GNET::Uncompress(os_com, os_src);
			return os_src;
		}
	};
public:
	class Storage;
	class ThreadContext
	{
		static pthread_key_t key;
		typedef std::vector<Storage *> StorageVec;
		StorageVec storage_vec;
		bool locked;
		bool cancel;
		size_t refcnt;
	public:
		ThreadContext () : locked(false), refcnt(0) { }
		Storage* AddStorage( Storage *storage )
		{
			//struct timeval tv;
    		//gettimeofday(&tv, NULL );
			//printf("ThreadContext(%d)::AddStorage %p, locked %d, refcnt %d, vecsize %d, time %d.%d;\n", 
			//	pthread_self(), storage, locked, refcnt , storage_vec.size(), tv.tv_sec, tv.tv_usec);
			storage_vec.push_back ( storage );
			return storage;
		}
		void lock();
		void unlock();
		void ref()     { refcnt++; 
			//printf("ThreadContext::ref, locked %d, refcnt %d, vecsize %d\n", locked, refcnt, storage_vec.size());
		}
		void release() { if ( --refcnt == 0 ) { unlock(); storage_vec.clear(); } }
		void set_cancel() { cancel = true; }
		static void destroy_context( void *p ) { delete (ThreadContext *)p; }
		static void init_key() { pthread_key_create(&key, &ThreadContext::destroy_context ); }
		static ThreadContext *GetInstance()
		{
			ThreadContext *spec = (ThreadContext *)pthread_getspecific(key);
			if ( spec )
				return spec;
			pthread_setspecific(key, spec = new ThreadContext());
			return spec;
		}
	};

	class Transaction
	{
		Transaction(const Transaction &rhs);
		void operator=(const Transaction &);
	public:
		~Transaction() { ThreadContext::GetInstance()->release(); }
		Transaction()  { ThreadContext::GetInstance()->ref(); }
		void lock() { ThreadContext::GetInstance()->lock(); }
		void commit() { }
		void abort( DbException e ) { ThreadContext::GetInstance()->set_cancel(); }
		void abort( ) { ThreadContext::GetInstance()->set_cancel(); }
	};

	typedef Transaction AtomTransaction;
	typedef Transaction CommonTransaction;

	class IQuery
	{
	public:
		virtual ~IQuery() { }
		virtual bool Update( Transaction &txn, GNET::Octets &key, GNET::Octets &val ) = 0;
	};

	class Storage
	{
		struct CompareOctets
		{
			bool operator() ( const GNET::Octets &o1, const GNET::Octets &o2 ) const
			{
				size_t s1 = o1.size();
				size_t s2 = o2.size();
				if ( int r = memcmp( o1.begin(), o2.begin(), std::min(s1, s2) ) )
					return r < 0;
				return s1 < s2;
			}
		};
		typedef std::map< const GNET::Octets, std::pair< void *, size_t >, CompareOctets > Prepared;
		typedef std::set< GNET::Octets, CompareOctets > NewKey;
		Prepared prepared;
		NewKey   new_key;
		char	 *path;
		GNET::DB *db;
		DataCoder *compressor;
		DataCoder *uncompressor;
		GNET::Thread::Mutex locker;
	public:
		~Storage()
		{
			delete compressor;
			delete uncompressor;
			free(path);
		}
		Storage( const char *file, GNET::DB *_db, DataCoder *c, DataCoder *u )
			: path(strdup(file)), db(_db), compressor(c), uncompressor(u), locker("Storage::locker") { }
		void SetCompressor( DataCoder *c, DataCoder *u )
		{
			delete compressor;
			delete uncompressor;
			compressor = c;
			uncompressor = u;
		}

		void snapshot_create()
		{
			db->snapshot_create();
			unlock();
		}
		void snapshot_release()
		{
			GNET::Thread::Mutex::Scoped l(locker);
			db->snapshot_release();
		}
		void lock() { 
		//printf("-------------storage lock %p\n", this);
		locker.Lock();     }
		void unlock() { 
		//printf("=============storage unlock %p\n", this);
		locker.UNLock(); }
		void checkpoint( bool cancel )
		{
			try
			{
				if ( cancel )
				{
					for ( Prepared::const_iterator it = prepared.begin(), ie = prepared.end(); it != ie; ++it )
					{
						db->put( (*it).first.begin(), (*it).first.size(), (*it).second.first, (*it).second.second );
						free ( (*it).second.first );
					}
					for ( NewKey::const_iterator it = new_key.begin(), ie = new_key.end(); it != ie; ++it )
						db->del( (*it).begin(), (*it).size() );
				}
				else
				{
					for ( Prepared::const_iterator it = prepared.begin(), ie = prepared.end(); it != ie; ++it )
						free ( (*it).second.first );
				}
				prepared.clear();
				new_key.clear();
				unlock();
			}
			catch ( GNET::__db_helper::PageFile::Exception e )
			{
				GNET::Log::log(LOG_ERR, "storagewdb::checkpoint DISK ERROR! PROGRAM ABORT!\n");
				//abort();
			}
		}

		void insert( const GNET::Octets &key, const GNET::Octets &val, Transaction &txn, int flags = 0 )
		{
			try
			{
				if( key.size() == 0 )
					throw DbException( WDB::WDB_KEYSIZE_ZERO );

				txn.lock();
				GNET::Octets com_val = compressor->Update(val);
				if ( flags & WDB_NOOVERWRITE )
				{
					if ( ! db->put( key.begin(), key.size(), com_val.begin(), com_val.size(), false ) )
						throw DbException( DbException::WDB_OVERWRITE );
					new_key.insert( key );	
				}
				else if ( prepared.find ( key ) == prepared.end() && new_key.find(key) == new_key.end() )
				{
					size_t val_len = com_val.size();
					if ( void *origin_val = db->put( key.begin(), key.size(), com_val.begin(), &val_len) )
						prepared.insert( std::make_pair( key, std::make_pair( origin_val, val_len ) ) );
					else
						new_key.insert( key );
				}
				else
					db->put( key.begin(), key.size(), com_val.begin(), com_val.size() );
			}
			catch ( DbException e )
			{
				txn.abort(e);
				throw e;
			}
			catch ( GNET::__db_helper::PageFile::Exception e )
			{
				GNET::Log::log(LOG_ERR, "storagewdb:insert DISK ERROR! PROGRAM ABORT!\n");
				//abort();
			}
		}

		GNET::Octets find( const GNET::Octets &key, Transaction &txn )
		{
			try
			{
				txn.lock();
				size_t val_len;
				if ( void *val = db->find( key.begin(), key.size(), &val_len ) )
				{
					GNET::Octets dbval = uncompressor->Update(GNET::Octets(val, val_len));
					free(val);
					return dbval;
				}
				throw DbException( DbException::WDB_NOTFOUND );
			}
			catch ( DbException e )
			{
				txn.abort(e);
				throw e;
			}
			catch ( GNET::__db_helper::PageFile::Exception e )
			{
				GNET::Log::log(LOG_ERR, "storagewdb:find DISK ERROR! PROGRAM ABORT!\n");
				//abort();
			}
			return GNET::Octets();
		}

		bool find( const GNET::Octets &key, GNET::Octets &val, Transaction &txn )
		{
			try
			{
				txn.lock();
				size_t val_len;
				if ( void *value = db->find( key.begin(), key.size(), &val_len ) )
				{
					uncompressor->Update(GNET::Octets(value, val_len)).swap(val);
					free(value);
					return true;
				}
				return false;
			}
			catch ( DbException e )
			{
				txn.abort(e);
				throw e;
			}
			catch ( GNET::__db_helper::PageFile::Exception e )
			{
				GNET::Log::log(LOG_ERR, "storagewdb:find DISK ERROR! PROGRAM ABORT!\n");
				//abort();
			}
			return false;
		}

		void del( const GNET::Octets &key, Transaction& txn, int flags = 0 )
		{
			try
			{
				txn.lock();
				if ( prepared.find ( key ) == prepared.end() )
				{
					size_t val_len;
					NewKey::iterator it = new_key.find( key );
					if ( it != new_key.end() )
					{
						new_key.erase(it);
						db->del ( key.begin(), key.size() );
					}
					else if ( void *origin_val = db->del ( key.begin(), key.size(), &val_len ) )
						prepared.insert( std::make_pair( key, std::make_pair( origin_val, val_len ) ) );
				}
				else
					db->del ( key.begin(), key.size() );
			}
			catch ( GNET::__db_helper::PageFile::Exception e )
			{
				GNET::Log::log(LOG_ERR, "storagewdb:del DISK ERROR! PROGRAM ABORT!\n");
				//abort();
			}
		}

		size_t count() const
		{
			return db->record_count();
		}

		class Cursor
		{
			Transaction *txn;
			const char  *path;
			GNET::DB *db;
			DataCoder *uncompressor;
			class CursorQuery : public GNET::IQueryData
			{
				IQuery *query;
				Cursor *cursor;
			public:
				CursorQuery( IQuery *q, Cursor *cur ) : query(q), cursor(cur) { }
				bool update( const void *key, size_t key_len, const void *val, size_t val_len )
				{
					GNET::Octets k(key, key_len);
					GNET::Octets v;
					try {
						v = cursor->uncompressor->Update(GNET::Octets(val, val_len));
					} catch ( GNET::Marshal::Exception e ) { }
					return query->Update( *cursor->txn, k, v );
				}
			};

			class CursorQueryRaw : public GNET::IQueryData
			{
				IQuery *query;
				Cursor *cursor;
			public:
				CursorQueryRaw( IQuery *q, Cursor *cur ) : query(q), cursor(cur) { }
				bool update( const void *key, size_t key_len, const void *val, size_t val_len )
				{
					GNET::Octets k(key, key_len);
					GNET::Octets v(val, val_len);
					return query->Update( *cursor->txn, k, v );
				}
			};
		public:
			Cursor( Transaction *_txn, const char *_path, GNET::DB *_db, DataCoder *u )
				: txn(_txn), path(_path), db(_db), uncompressor(u) { }

			void walk( IQuery &query )
			{
				try
				{
					CursorQuery q( &query, this );
					txn->lock();
					db->walk( &q );
				}
				catch ( GNET::__db_helper::PageFile::Exception e )
				{
					GNET::Log::log(LOG_ERR, "storagewdb:walk DISK ERROR! PROGRAM ABORT!\n");
					//abort();
				}
			}

			void walk( GNET::Octets &begin, IQuery &query )
			{
				try
				{
					CursorQuery q( &query, this );
					txn->lock();
					db->walk( begin.begin(), begin.size(), &q );
				}
				catch ( GNET::__db_helper::PageFile::Exception e )
				{
					GNET::Log::log(LOG_ERR, "storagewdb:walk DISK ERROR! PROGRAM ABORT!\n");
					//abort();
				}
			}

			void walk_raw( IQuery &query )
			{
				try
				{
					CursorQueryRaw q( &query, this );
					txn->lock();
					db->walk( &q );
				}
				catch ( GNET::__db_helper::PageFile::Exception e )
				{
					GNET::Log::log(LOG_ERR, "storagewdb:walk_raw DISK ERROR! PROGRAM ABORT!\n");
					//abort();
				}
			}

			void walk_raw( GNET::Octets &begin, IQuery &query )
			{
				try
				{
					CursorQueryRaw q( &query, this );
					txn->lock();
					db->walk( begin.begin(), begin.size(), &q );
				}
				catch ( GNET::__db_helper::PageFile::Exception e )
				{
					GNET::Log::log(LOG_ERR, "storagewdb:walk_raw DISK ERROR! PROGRAM ABORT!\n");
					//abort();
				}
			}

			void browse( IQuery &query )
			{
				try
				{
					CursorQuery q( &query, this );
					GNET::PageBrowser( path ).action(&q);
				}
				catch ( GNET::__db_helper::PageFile::Exception e )
				{
					GNET::Log::log(LOG_ERR, "storagewdb:browse DISK ERROR! PROGRAM ABORT!\n");
					//abort();
				}
			}

			void browse_raw( IQuery &query )
			{
				try
				{
					CursorQueryRaw q( &query, this );
					GNET::PageBrowser( path ).action(&q);
				}
				catch ( GNET::__db_helper::PageFile::Exception e )
				{
					GNET::Log::log(LOG_ERR, "storagewdb:browse_raw DISK ERROR! PROGRAM ABORT!\n");
					//abort();
				}

			}
		};

		Cursor cursor( Transaction &txn ) { return Cursor( &txn, path, db, uncompressor ); }
	};
private:
	typedef std::map< std::string, Storage * > StorageMap;
	typedef std::vector<Storage *> StorageVec;
	static StorageMap storage_map;
	static StorageVec storage_vec;
	static GNET::DBCollection *env;
	static std::string homedir;
	static std::string datadir;
	static std::string logdir;
	static GNET::Thread::Mutex checkpoint_locker;
public:
	static void writecompressstatus( bool __iscompress )
	{
		// do nothing
	}

	static Storage *GetStorage( const char *dbfile )
	{
		StorageMap::iterator it = storage_map.find( dbfile );
		return it != storage_map.end() ? ThreadContext::GetInstance()->AddStorage( (*it).second ) : NULL;
	}

	static bool Open()
	{
		GNET::Conf *conf = GNET::Conf::GetInstance();

		homedir = conf->find( "storagewdb", "homedir" );
		datadir = conf->find( "storagewdb", "datadir" );
		logdir = conf->find( "storagewdb", "logdir" );

		if( homedir.length() == 0 ) homedir = WDB_HOMEDIR_DEFAULT;
		if( datadir.length() == 0 ) datadir = WDB_DATADIR_DEFAULT;
		if( logdir.length() == 0 )  logdir = WDB_LOGDIR_DEFAULT;

		int len = homedir.length();
		if ( len > 0 && (homedir[len-1] == '\\' || homedir[len-1] == '/') )
			homedir.erase( homedir.end() - 1 );
		len = datadir.length();
		if ( len > 0 && (datadir[len-1] == '\\' || datadir[len-1] == '/') )
			datadir.erase( datadir.end() - 1 );
		len = logdir.length();
		if ( len > 0 && (logdir[len-1] == '\\' || logdir[len-1] == '/') )
			logdir.erase( logdir.end() - 1 );
		if ( datadir.length() > 0 && '/' != datadir[0] )
			datadir = homedir + "/" + datadir;
		if ( logdir.length() > 0 && '/' != logdir[0] )
			logdir = homedir + "/" + logdir;

		ThreadContext::init_key();
		env = new GNET::DBCollection();

		env->set_data_dir( datadir.c_str() );
		env->set_log_dir ( logdir.c_str() );

		size_t cache_high_default = atoi( conf->find( "storagewdb", "cache_high_default" ).c_str() );
		size_t cache_low_default = atoi( conf->find( "storagewdb", "cache_low_default" ).c_str() );
		std::vector<std::string>	tables;
		if( GNET::ParseStrings( conf->find( "storagewdb", "tables" ), tables ) )
		{
			for( size_t k=0; k<tables.size(); k++ )
			{
				size_t cache_high = cache_high_default;
				size_t cache_low = cache_low_default;
				std::string str_high = conf->find( "storagewdb", tables[k] + "_cache_high" );
				std::string str_low  = conf->find( "storagewdb", tables[k] + "_cache_low" );
				if( str_high.length() > 0 )	cache_high = atoi( str_high.c_str() );
				if( str_low.length() > 0 )	cache_low  = atoi( str_low.c_str() );
				if( 0 == cache_high )	cache_high = cache_high_default;
				if( 0 == cache_low )	cache_low  = cache_low_default;
				env->set_table( tables[k].c_str(), cache_high, cache_low );
			}
		}

		if ( env->init() )
		{
			for( size_t k=0; k<tables.size(); k++ )
			{
				Storage *storage = new Storage((datadir + "/" + tables[k]).c_str(), env->db(tables[k].c_str()),new Compressor(),new Uncompressor());
				//printf(">>>>>>>>>>>storage init table %s is %p\n", tables[k].c_str(), storage);
				storage_map.insert( std::make_pair(tables[k], storage ) );
				storage_vec.push_back( storage );
			}
			std::sort ( storage_vec.begin(), storage_vec.end() );
			return true;
		}
		return false;
	}

	static std::string get_datadir()
	{
		return datadir;
	}

	static std::string get_logdir()
	{
		return logdir;
	}

	static void Close()
	{
		GNET::Thread::Mutex::Scoped l(checkpoint_locker);

		if( env )
		{
			delete env;
			env = NULL;
		}
		for( StorageVec::iterator it = storage_vec.begin(), ite = storage_vec.end(); it != ite; ++ it )
			delete (*it);
		storage_map.clear();
		storage_vec.clear();
	}

	static bool checkpoint()
	{
		try
		{
			// avoid checkpoint reentrance
			GNET::Thread::Mutex::Scoped l(checkpoint_locker);
			if (NULL == env) return false; // evn hasclosed

			std::for_each( storage_vec.begin(), storage_vec.end(), std::mem_fun(&Storage::lock) );
			std::for_each( storage_vec.begin(), storage_vec.end(), std::mem_fun(&Storage::snapshot_create) );
			if ( ! env->checkpoint_prepare() )
			{
				std::for_each( storage_vec.begin(), storage_vec.end(), std::mem_fun(&Storage::snapshot_release) );
				return false;
			}
			bool r = env->checkpoint_commit();
			std::for_each( storage_vec.begin(), storage_vec.end(), std::mem_fun(&Storage::snapshot_release) );
			return r;
		}
		catch ( GNET::__db_helper::PageFile::Exception e )
		{
			GNET::Log::log(LOG_ERR, "storagewdb:checkpoint DISK ERROR! PROGRAM ABORT!\n");
			//abort();
		}
		return false;
	}

	static void removeoldlogs( )
	{
		if( env )
		{
			std::vector<time_t> r = env->remove_logs();
			char *name = (char *)malloc( logdir.length() + 32 );
			std::vector<time_t>::iterator it, ite;
			for( it = r.begin(), ite = r.end(); it != ite; ++it )
			{
				sprintf(name, "%s/log.%08lx", logdir.c_str(), *it );
				unlink( name );
			}
			free(name);
		}
	}

	static void backup( const char * __destdir, bool increment = false )
	{
		time_t t = time(NULL);
		struct tm * ptm = localtime( &t );
		char buffer[32];
		sprintf( buffer, "%.4d%.2d%.2d-%.2d%.2d%.2d",
			ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,
			ptm->tm_hour, ptm->tm_min, ptm->tm_sec );

		std::string destdir = __destdir;
		int len = destdir.length();
		if( len>0 && destdir[len-1] != '/' )
			destdir += "/";
		mkdir( destdir.c_str(), 0755 );
		destdir += buffer;
		mkdir( destdir.c_str(), 0755 );

		std::string scmd = "/bin/cp -r ";
		if( !increment )
		{
			scmd += datadir + " " + destdir + "/";
			scmd += ";/bin/cp -r ";
		}
		scmd += logdir + " " + destdir + "/";
		system( scmd.c_str() );
	}

	static void * BackupThread( void * pParam )
	{
		pthread_detach( pthread_self() );

		sigset_t sigs;
		sigfillset(&sigs);
		pthread_sigmask(SIG_BLOCK, &sigs, NULL);

		static unsigned int times = 0;
		static unsigned int times2 = 0;
		time_t last_checked = time(NULL);

		while( true )
		{
			try
			{
				GNET::Conf *conf = GNET::Conf::GetInstance();
				long cp_interval = atol( conf->find( "storagewdb", "checkpoint_interval" ).c_str() );
				std::string backup_lockfile = conf->find( "storagewdb", "backup_lockfile" );
				std::string quit_lockfile = conf->find( "storagewdb", "quit_lockfile" );
				long times_incbackup = atol( conf->find( "storagewdb", "times_incbackup" ).c_str() );

				int elapsed = time(NULL) - last_checked;
				if ( elapsed < cp_interval )
					sleep( cp_interval - elapsed );
				last_checked = time(NULL);

				LOG_TRACE( "checkpoint begin." );
				if ( !StorageEnv::checkpoint() )
				{
					GNET::Log::log(LOG_ERR, "storagewdb:BackupThread DISK ERROR! PROGRAM ABORT!\n");
					//abort();
				}
				LOG_TRACE( "checkpoint end." );

				if( 0 == unlink(quit_lockfile.c_str()) )
				{
					LOG_TRACE( "quit." );
					if( 0 != kill( 0, SIGKILL ) )	exit(0);
				}

				if( 1 == times_incbackup && !((++times2)%60) )
					StorageEnv::removeoldlogs( );

				if( 0 == unlink(backup_lockfile.c_str()) )
				{
					times ++;
					std::string backupdir = conf->find( "storagewdb", "backupdir" );
					if( !((times-1)%times_incbackup) )
					{
						LOG_TRACE( "backup all begin (times=%u).", times );
						StorageEnv::removeoldlogs( );
						StorageEnv::backup( backupdir.c_str() );
					}
					else
					{
						LOG_TRACE( "backup increment begin (times=%u).", times );
						StorageEnv::backup( backupdir.c_str(), true );
					}
					LOG_TRACE( "backup end (times=%u).", times );
				}
			}
			catch( ... ) {  }
		}

		return NULL;
	}
};

inline void StorageEnv::ThreadContext::lock()
{
	//printf("ThreadContext(%d)::lock, locked %d, refcnt %d, vecsize %d\n", pthread_self(), locked, refcnt, storage_vec.size());
	if ( locked ) return;
	std::sort( storage_vec.begin(), storage_vec.end() );
	std::for_each( storage_vec.begin(), storage_vec.end(), std::mem_fun(&Storage::lock) );
	locked = true;
	cancel = false;
}

inline void StorageEnv::ThreadContext::unlock( )
{
	//printf("ThreadContext(%d)::unlock, locked %d, refcnt %d, vecsize %d\n", pthread_self(), locked, refcnt, storage_vec.size());
	if ( !locked ) return;
	std::for_each( storage_vec.begin(), storage_vec.end(), std::bind2nd( std::mem_fun(&Storage::checkpoint), cancel) );
	//storage_vec.clear();
	locked = false;
			//struct timeval tv;
    		//gettimeofday(&tv, NULL );
	//printf("ThreadContext::unlock over, locked %d, refcnt %d, vecsize %d, time %d.%d\n", locked, refcnt, storage_vec.size(), tv.tv_sec, tv.tv_usec);
}

};

#endif

