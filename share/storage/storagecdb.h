#ifndef __STORAGECDB_H
#define __STORAGECDB_H

#include <db_cxx.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <typeinfo>

#include "octets.h"
#include "conf.h"
#include "mutex.h"
#include "compress.h"
#include "reference.h"

#define CDB_HOMEDIR_DEFAULT "./dbhome"
#define CDB_DATADIR_DEFAULT "./dbdata"
#define CDB_LOGDIR_DEFAULT  "./dblogs"

#define CDB_SIZE_PAGE 4096
#define CDB_SIZE_CACHE_DEFAULT 64 * 1048576

#define CDBLOG_EXCEPTION(prefix) Log::log( LOG_ERR, prefix":%s", e.what() );
#define CDBLOG_DBEXCEPTION(prefix,dbname) Log::log( LOG_ERR, "%s(%s):%s", prefix, dbname.c_str(), e.what() );

#ifndef STRICTNAMESPACE
using namespace GNET;
#endif

namespace CDB
{
class StorageEnv 
{
public:
	struct DataCoder
	{
		virtual ~DataCoder() { }
		virtual Octets Update (Octets os) = 0;
	};

	struct NullCoder : public DataCoder
	{
		Octets Update (Octets os) { return os; }
	};

	struct Compressor : public DataCoder
	{
		Octets Update (Octets os_src)
		{
			Octets os_com;
			GNET::Compress(os_src, os_com);
			return os_com;
		}
	};

	struct Uncompressor : public DataCoder
	{
		Octets Update (Octets os_com)
		{
			Octets os_src;
			GNET::Uncompress(os_com, os_src);
			return os_src;
		}
	};

	struct DBData : public Dbt
	{
		operator Octets() const { return Octets(get_data(), get_size()); }
	};

	struct DBConstData : public DBData
	{
		DBConstData( const Octets &data )
		{
			set_data(const_cast<void*>(data.begin()));
			set_size(data.size());
		}
	};

	struct DBMallocData : public DBData
	{
		~DBMallocData() { free( get_data() ); }
		DBMallocData() { set_data(NULL); set_flags(DB_DBT_MALLOC); }
	};

	struct DBUserData : public DBData
	{
		~DBUserData() { free( get_data() ); }
		DBUserData( size_t npage )
		{
			set_flags(DB_DBT_USERMEM);
			set_data(malloc(CDB_SIZE_PAGE * npage));
			set_ulen(CDB_SIZE_PAGE * npage);
		}
	};

	class Transaction
	{
	public:
		virtual ~Transaction() { }
		virtual void abort(const DbException& e) { }
		virtual operator DbTxn *() = 0;
		virtual int WFLAG(int flag) const { return flag; }
		virtual int RFLAG(int flag) const { return flag; }
	};

	class NullTransaction : public Transaction
	{
	public:
		operator DbTxn *() { return NULL; }
	};

	class IQuery
	{
	public:
		virtual ~IQuery() { }
		virtual bool Update( Transaction& txn, Octets& key, Octets& value ) = 0;
	};

	class Storage
	{
		struct CompareOctets
		{
			bool operator() ( const Octets& o1, const Octets &o2 ) const
			{
				size_t s1 = o1.size();
				size_t s2 = o2.size();
				if ( s1 == s2 )
					return memcmp( o1.begin(), o2.begin(), s1 ) < 0;
				return s1 < s2;
			}
		};
		typedef std::set< Octets, CompareOctets > KeyChange;
		typedef std::map< const Octets, Octets, CompareOctets> Cache;
		Cache cache;
		Cache saved;
		KeyChange keychange;
		Db *db;
		std::string name;
		HardReference<DataCoder> compressor;
		HardReference<DataCoder> uncompressor;
		Thread::Mutex locker;
	public:
		~Storage()
		{
			if ( db )
			{
				try
				{
					checkpoint();
					db->close(0);
				}
				catch ( DbException e )
				{
					CDBLOG_DBEXCEPTION("Storage::~Storage",name)
				}
				delete db;
			}
		}

		Storage( DbEnv *env, const char *__dbname, DataCoder* c, DataCoder* u )
			: db ( new Db( env, 0 ) ), name(__dbname), compressor(c), uncompressor(u),
			locker("StorageEnv::Storage::locker")
		{
			try
			{
				db->set_pagesize(CDB_SIZE_PAGE);
				u_int32_t flags = DB_CREATE;
#if defined _REENTRANT
				flags |= DB_THREAD;
#endif
				AtomTransaction atom_txn;
				db->open(atom_txn, __dbname, NULL, DB_BTREE, atom_txn.WFLAG(flags), 0664);
			}
			catch ( DbException e )
			{
				delete db;
				db = NULL;
				CDBLOG_DBEXCEPTION("Storage::Storage",name)
				throw;
			}
		}

		void SetCompressor( DataCoder *c, DataCoder *u )
		{
			compressor = HardReference<DataCoder>(c);
			uncompressor = HardReference<DataCoder>(u);
		}

		void lock() { locker.Lock(); }
		void unlock() { locker.UNLock(); }
		void commit() { keychange.clear(); saved.clear(); }
		void abort()
		{
			for ( KeyChange::const_iterator it = keychange.begin(), ie = keychange.end(); it != ie; ++it )
				cache.erase( *it );
			cache.insert( saved.begin(), saved.end() );
			commit();
		}

		void checkpoint()
		{
			DbTxn *txn;
			try
			{
				Thread::Mutex::Scoped l(locker);
				env->txn_begin( NULL, &txn, 0 );
				for ( Cache::const_iterator it = cache.begin(), ie = cache.end(); it != ie; ++it )
				{
					Octets com_val = compressor->Update((*it).second);
					DBConstData dbkey((*it).first);
					DBConstData dbval(com_val);
					if ( int rv = db->put(txn, &dbkey, &dbval, 0) )
						throw DbException(rv);
				}
				txn->commit(0);
				cache.clear();
			}
			catch ( DbException e )
			{
				txn->abort();
				CDBLOG_DBEXCEPTION("Storage::checkpoint",name)
			}
		}

		void insert( const Octets& key, const Octets& val, Transaction& txn, int flags = 0 )
		{
			try
			{
				if ( flags & DB_NOOVERWRITE )
				{
					Octets old_value;
					if ( find( key, old_value, txn, 0 ) ) throw DbException(EINVAL);
				}
				if ( typeid( txn ) == typeid(CommonTransaction) )
				{
					if ( keychange.find(key) == keychange.end() )
					{
						keychange.insert( key );
						Cache::iterator it = cache.find(key);
						if ( it != cache.end() )
							saved.insert ( *it );
					}
					cache.insert( std::make_pair( key, val ) );
				}
				else
				{
					Thread::Mutex::Scoped l(locker);
					cache.insert( std::make_pair( key, val ) );
				}
			}
			catch ( DbException e )
			{
				txn.abort(e);
				CDBLOG_DBEXCEPTION("Storage::insert",name)
				throw;
			}
		}

		void insert_nocompress( const Octets& key, const Octets& val, Transaction& txn, int flags = 0 )
		{
			try
			{
				if ( cache.find(key) != cache.end() )
					throw DbException(EINVAL);
				DBConstData dbkey(key);
				DBConstData dbval(val);
				if ( int rv = db->put(txn, &dbkey, &dbval, txn.WFLAG(flags)) )
					throw DbException(rv);
			}
			catch ( DbException e )
			{
				txn.abort(e);
				CDBLOG_DBEXCEPTION("Storage::insert_nocompress",name)
				throw;
			}
		}

		Octets find( const Octets& key, Transaction& txn, int flags = 0 )
		{
			try
			{
				Cache::iterator it = cache.find(key);
				if ( it != cache.end() ) return (*it).second;
				DBConstData dbkey(key);
				DBMallocData dbval;
				if ( int rv = db->get(txn, &dbkey, &dbval, txn.RFLAG(flags)) )
					throw DbException(rv);
				return uncompressor->Update(dbval);
			}
			catch ( DbException e )
			{
				txn.abort(e);
				if( DB_NOTFOUND != e.get_errno() )
					CDBLOG_DBEXCEPTION("Storage::find",name)
				throw;
			}
		}

		bool find( const Octets& key, Octets& val, Transaction& txn, int flags = 0 )
		{
			try
			{
				Cache::iterator it = cache.find(key);
				if ( it == cache.end() )
				{
					DBConstData dbkey(key);
					DBMallocData dbval;
					if ( int rv = db->get(txn, &dbkey, &dbval, txn.RFLAG(flags)) )
						if ( rv == DB_NOTFOUND )
							return false;
						else
							throw DbException(rv);
					uncompressor->Update(dbval).swap(val);
				}
				else
					val = (*it).second;
				return true;
			}
			catch ( DbException e )
			{
				txn.abort(e);
				throw;
			}
		}

		bool findcache( const Octets& key, Octets& val )
		{
			Cache::iterator it = cache.find(key);
			if ( it == cache.end() ) return false;
			val = (*it).second;
			return true;
		}

		Octets find_nocompress( const Octets& key, Transaction& txn, int flags = 0 )
		{
			try
			{
				DBConstData dbkey(key);
				DBMallocData dbval;
				if ( int rv = db->get(txn, &dbkey, &dbval, txn.RFLAG(flags)) )
					throw DbException(rv);
				return dbval;
			}
			catch ( DbException e )
			{
				txn.abort(e);
				if( DB_NOTFOUND != e.get_errno() )
					CDBLOG_DBEXCEPTION("Storage::find_nocompress",name)
				throw;
			}
		}

		void del( const Octets &key, Transaction& txn, int flags = 0 )
		{
			try
			{
				DBConstData dbkey(key);
				db->del(txn, &dbkey, txn.WFLAG(flags));
				if ( typeid(txn) == typeid(CommonTransaction) )
				{
					keychange.erase( key );
					saved.erase( key );
				}
				cache.erase( key );
			}
			catch ( DbException e )
			{
				txn.abort(e);
				CDBLOG_DBEXCEPTION("Storage::del",name)
				throw;
			}
		}

		class Cursor
		{
			Storage *parent;
			Transaction *txn;
			Dbc *dbc;
		public:
			~Cursor()
			{
				try
				{
					dbc->close();
				}
				catch ( DbException e )
				{
					txn->abort(e);
					CDBLOG_DBEXCEPTION("Storage::Cursor::~Cursor",parent->name)
					throw;
				}
			}
			Cursor( Storage *__parent, Transaction& __txn, Dbc* __dbc )
				: parent(__parent), txn(&__txn), dbc(__dbc) { }
			void walk( IQuery &query, int flags = 0, size_t npage = 256 )
			{
				try
				{
					DBMallocData dbkey;
					DBUserData dbval(npage);
					Octets key, val;
					Dbt ik, iv;
					while ( true )
					{
						if ( int rv = dbc->get( &dbkey, &dbval, DB_NEXT|DB_MULTIPLE_KEY))
						{
							if ( rv != DB_NOTFOUND )
								throw DbException(rv);
							return;
						}
						DbMultipleKeyDataIterator it(dbval);
						while ( it.next(ik, iv) )
						{
							key.replace(ik.get_data(), ik.get_size()); 
							if ( !parent->findcache( key, val ) )
								val = parent->uncompressor->Update( Octets(iv.get_data(), iv.get_size()));
							if ( !query.Update( *txn, key, val ))
								return;
						}
					}
				}
				catch ( DbException e )
				{
					txn->abort(e);
					CDBLOG_DBEXCEPTION("Storage::walk",parent->name)
					throw;
				}
			}

			void walk( Octets &begin, IQuery &query, int flags = 0, size_t npage = 256 )
			{
				try
				{
					DBConstData dbkey(begin);
					DBUserData dbval(npage);
					Octets key, val;
					Dbt ik, iv;
					bool bfirst = true;
					while ( true )
					{
						int rv;
						if( !bfirst )
							rv = dbc->get( &dbkey, &dbval, DB_NEXT|DB_MULTIPLE_KEY );
						else if( 0 == begin.size() )
							rv = dbc->get( &dbkey, &dbval, DB_FIRST|DB_MULTIPLE_KEY );
						else
							rv = dbc->get( &dbkey, &dbval, DB_SET_RANGE|DB_MULTIPLE_KEY );
						bfirst = false;

						if ( rv )
						{
							if ( rv != DB_NOTFOUND )
								throw DbException(rv);
							return;
						}
						DbMultipleKeyDataIterator it(dbval);
						while ( it.next(ik, iv) )
						{
							key.replace(ik.get_data(), ik.get_size()); 
							if ( !parent->findcache( key, val ) )
								val = parent->uncompressor->Update( Octets(iv.get_data(), iv.get_size()));
							if ( !query.Update( *txn, key, val ))
							{
								begin.replace(ik.get_data(), ik.get_size());
								return;
							}
						}
						begin.clear();
					}
				}
				catch ( DbException e )
				{
					txn->abort(e);
					CDBLOG_DBEXCEPTION("Storage::walk",parent->name)
					throw;
				}
			}

			void browse( IQuery &query )
			{
				walk( query );
			}

		};

		Cursor cursor( Transaction& txn, int flags = 0 )
		{
			try
			{
				Dbc *dbc;
				db->cursor(txn, &dbc, flags);
				return Cursor( this, txn, dbc );
			}
			catch ( DbException e )
			{
				txn.abort(e);
				CDBLOG_DBEXCEPTION("Storage::cursor",name)
				throw;
			}
		}

		int count( int flags = 0 )
		{
			int count = -1;
			try
			{
				DB_BTREE_STAT * pstat = NULL;
				db->stat(&pstat, flags);
				if ( pstat )
				{
					count = pstat->bt_nkeys;
					free(pstat);
				}
			}
			catch ( DbException e )
			{
				CDBLOG_DBEXCEPTION("Storage::count",name)
			}
			return count;
		}

		void sync( int flags = 0 )
		{
			try
			{
				db->sync(flags);
			}
			catch ( DbException e )
			{
				CDBLOG_DBEXCEPTION("Storage::sync",name)
			}
		}
	};

	class AtomTransaction : public Transaction
	{
	public:
		operator DbTxn *() { return NULL; }
		int WFLAG(int flag) const { return flag | DB_AUTO_COMMIT; }
	};

	class CommonTransaction : public Transaction
	{
		void unlock()
		{
			std::for_each ( storages.rbegin(), storages.rend(), std::mem_fun(&Storage::unlock));
		}

		void lock( int flags = 0 ) 
		{
			std::sort( storages.begin(), storages.end() );
			std::for_each ( storages.begin(), storages.end(), std::mem_fun(&Storage::lock));
			try
			{
				env->txn_begin( NULL, &txn, flags );
				stage = STAGE_FINE;
			}
			catch ( DbException e )
			{
				CDBLOG_EXCEPTION("Transaction::txn_begin")
				throw;
			}
		}
	protected:
		enum { STAGE_INIT, STAGE_FINE, STAGE_ERROR } stage;
		DbTxn *txn;
		DbException exception;
		std::vector<Storage *> storages;
	public:
		CommonTransaction( Storage *s1, Storage *s2, int flags = 0 )
			: stage(STAGE_INIT), txn(NULL), exception(0)
		{
			storages.push_back ( s1 );
			storages.push_back ( s2 );
			lock();
		}
		
		CommonTransaction( Storage *s1, Storage *s2, Storage *s3, int flags = 0 )
			: stage(STAGE_INIT), txn(NULL), exception(0)
		{
			storages.push_back ( s1 );
			storages.push_back ( s2 );
			storages.push_back ( s3 );
			lock();
		}
		CommonTransaction( Storage *s1, Storage *s2, Storage *s3, Storage *s4, int flags = 0 )
			: stage(STAGE_INIT), txn(NULL), exception(0)
		{
			storages.push_back ( s1 );
			storages.push_back ( s2 );
			storages.push_back ( s3 );
			storages.push_back ( s4 );
			lock();
		}

		~CommonTransaction ()
		{
			try
			{
				switch ( stage )
				{
				case STAGE_FINE: 
					txn->commit(0);
					std::for_each ( storages.begin(), storages.end(), std::mem_fun(&Storage::commit));
					break;
				case STAGE_ERROR:
					txn->abort();
					std::for_each ( storages.begin(), storages.end(), std::mem_fun(&Storage::abort));
					break;
				default:;
				}
			}
			catch ( DbException e )
			{
				CDBLOG_EXCEPTION("CommonTransaction::~CommonTransaction")
			}
			unlock();
		}

		void abort(const DbException& e)
		{
			if ( stage == STAGE_FINE )
			{
				exception = e;
				stage = STAGE_ERROR;
			}
		}

		operator DbTxn *()
		{
			if ( stage == STAGE_ERROR )
				throw DbException(exception);
			return txn;
		}
	};
	typedef std::map<std::string, Storage *> StorageMap;
private:
	static StorageMap smap;
	static Thread::Mutex locker_smap;
	static DbEnv *env;

	static std::string homedir;
	static std::string datadir;
	static std::string datadirorg;
	static std::string logdir;
	static std::string logdirorg;
	static bool iscompress;

	StorageEnv() { }
	static void err_callback( const char *errpfx, char *msg )
	{
	}

	static void readcompressstatus( )
	{
		Storage config(env, STORAGE_CONFIGDB, new NullCoder(), new NullCoder() );
		Marshal::OctetsStream os_key, os_value;
		int index = 1;
		os_key << index;

		bool success = false;
		try
		{
			StorageEnv::AtomTransaction	txn;
			try
			{
				os_value = config.find_nocompress( os_key, txn );
				os_value >> iscompress;
				success = true;
				std::cerr << "read compress status from storage, compress="<<iscompress<<std::endl;
			}
			catch ( DbException e ) { throw; }
			catch ( ... )
			{
				DbException ee( DB_OLD_VERSION );
				txn.abort( ee );
				throw ee;
			}
		}
		catch ( DbException e ) { }

		if( !success )
		{
			GNET::Conf *conf = GNET::Conf::GetInstance();
			iscompress = (atoi(conf->find( "storage", "compress" ).c_str()) ? true : false);
			std::cerr << "read compress status from config file, compress="<<iscompress<<std::endl;
			os_value << iscompress;
			try
			{
				StorageEnv::AtomTransaction	txn;
				try
				{
					config.insert_nocompress( os_key, os_value, txn );
				}
				catch ( DbException e ) { throw; }
				catch ( ... )
				{
					DbException ee( DB_OLD_VERSION );
					txn.abort( ee );
					throw ee;
				}
			}
			catch ( DbException e ) { CDBLOG_EXCEPTION( "StorageEnv::readcompressstatus" ) }
		}
	}
public:
	static void writecompressstatus( bool __iscompress )
	{
		Storage config( env, STORAGE_CONFIGDB, new NullCoder(), new NullCoder() );
		Marshal::OctetsStream os_key, os_value;
		int index = 1;
		os_key << index;
		os_value << __iscompress;

		try
		{
			StorageEnv::AtomTransaction	txn;
			try
			{
				config.insert_nocompress( os_key, os_value, txn );
			}
			catch ( DbException e ) { throw; }
			catch ( ... )
			{
				DbException ee( DB_OLD_VERSION );
				txn.abort( ee );
				throw ee;
			}
		}
		catch ( DbException e ) { CDBLOG_EXCEPTION( "StorageEnv::writecompressstatus" ) }
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
		if ( env )
		{
			for ( StorageMap::iterator it = smap.begin(), ie = smap.end(); it != ie; ++it )
				delete (*it).second;
			smap.clear();
			try
			{
				env->close(0);
			}
			catch ( DbException e )
			{
				CDBLOG_EXCEPTION( "StorageEnv::Close" )
			}
			delete env;
			env = NULL;
		}
	}

	static void Open(bool bJoinEnv = false)
	{
		GNET::Conf *conf = GNET::Conf::GetInstance();

		std::string errpfx;
		homedir = conf->find( "storage", "homedir" );
		datadir = conf->find( "storage", "datadir" );
		logdir = conf->find( "storage", "logdir" );
		errpfx = conf->find( "storage", "errpfx" );
		u_int32_t cachesize = atol( (conf->find( "storage", "cachesize" )).c_str() );

		if( homedir.length() == 0 ) homedir = CDB_HOMEDIR_DEFAULT;
		if( datadir.length() == 0 ) datadir = CDB_DATADIR_DEFAULT;
		if( logdir.length() == 0 )  logdir = CDB_LOGDIR_DEFAULT;
		if( errpfx.length() == 0 )  errpfx = "Storage";
		if( cachesize <= 0 )		cachesize = CDB_SIZE_CACHE_DEFAULT;

		int len = homedir.length();
		if ( len > 0 && (homedir[len-1] == '\\' || homedir[len-1] == '/') )
			homedir.erase( homedir.end() - 1 );
		len = datadir.length();
		if ( len > 0 && (datadir[len-1] == '\\' || datadir[len-1] == '/') )
			datadir.erase( datadir.end() - 1 );
		datadirorg = datadir;
		len = logdir.length();
		if ( len > 0 && (logdir[len-1] == '\\' || logdir[len-1] == '/') )
			logdir.erase( logdir.end() - 1 );
		logdirorg = logdir;
		if ( datadir.length() > 0 && '/' != datadir[0] )
			datadir = homedir + "/" + datadir;
		if ( logdir.length() > 0 && '/' != logdir[0] )
			logdir = homedir + "/" + logdir;

		u_int32_t flags = DB_CREATE | DB_INIT_LOCK | DB_INIT_MPOOL | DB_INIT_TXN;
		flags |= ( bJoinEnv ? DB_JOINENV | DB_USE_ENVIRON : DB_PRIVATE );
#if defined _REENTRANT
		flags |= DB_THREAD;
#endif
		mkdir(homedir.c_str(), 0755);
		mkdir(datadir.c_str(), 0755);
		mkdir(logdir.c_str(), 0755);

		env = new DbEnv(0);
		try
		{
			env->set_errpfx( errpfx.c_str() );
			env->set_errcall( err_callback );
			env->set_cachesize(0, cachesize, 0);
			env->set_data_dir( datadirorg.c_str() );
			env->set_lg_dir( logdirorg.c_str() );
			env->set_timeout(10000,DB_SET_LOCK_TIMEOUT);
			env->set_timeout(10000,DB_SET_TXN_TIMEOUT);
			env->set_lk_detect(DB_LOCK_DEFAULT);
			env->open(homedir.c_str(), flags, 0 );
		}
		catch ( DbException e )
		{
			delete env;
			env = NULL;
			CDBLOG_EXCEPTION("StorageEnv::Open")
		}

		readcompressstatus( );
	}

	static Storage *GetStorage( const char *__dbname = "storage" )
	{
		Thread::Mutex::Scoped l(locker_smap);
		StorageMap::iterator it = smap.find( __dbname );
		if ( it != smap.end() )
			return (*it).second;
		return (*smap.insert( it, std::make_pair( __dbname, new Storage(env, __dbname, iscompress ? (DataCoder*)new Compressor() : (DataCoder*)new NullCoder(), iscompress ? (DataCoder*)new Uncompressor() : (DataCoder*)new NullCoder() ) ) )).second;
	}

	static void checkpoint()
	{
		try
		{
			std::vector<Storage *> storages;
			for ( StorageMap::iterator it = smap.begin(), ie = smap.end(); it != ie; ++it ) storages.push_back ( (*it).second );
			std::sort ( storages.begin(), storages.end() );
			std::for_each ( storages.begin(), storages.end(), std::mem_fun(&Storage::checkpoint) );
			env->txn_checkpoint( 0, 0, DB_FORCE );
		}
		catch ( DbException e )
		{
			CDBLOG_EXCEPTION("StorageEnv::checkpoint")
			Close();
			Open();
		}
	}

	static void removeoldlogs( )
	{
		try
		{
			char **begin = NULL, **list = NULL;
			int ret = env->log_archive(&list, DB_ARCH_ABS /*|DB_ARCH_LOG*/);
			if( 0 != ret )
				Log::log( LOG_ERR, "StorageEnv::removeoldlogs,log_archive ret=%d,error=%s",
						ret, DbEnv::strerror(ret) );
			if( 0 == ret && list != NULL )
			{
				for( begin = list; *list != NULL; ++list)
				{
					if( *list != NULL )
						unlink( *list );
				}
				free(begin);
			}
		}
		catch ( DbException e )
		{
			CDBLOG_EXCEPTION("StorageEnv::removeoldlogs")
		}
	}

	static void backup( const char * __destdir )
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
		scmd += datadir + " " + destdir + "/";
		scmd += ";/bin/cp -r ";
		scmd += logdir + " " + destdir + "/";
		system( scmd.c_str() );
	}

	static void * BackupThread( void * pParam )
	{
		pthread_detach( pthread_self() );

		sigset_t sigs;
		sigfillset(&sigs);
		pthread_sigmask(SIG_BLOCK, &sigs, NULL);
		time_t last_checked = time(NULL);

		while( true )
		{
			try
			{
				GNET::Conf *conf = GNET::Conf::GetInstance();
				long cp_interval = atol( conf->find( "storage", "checkpoint_interval" ).c_str() );
				std::string backup_lockfile = conf->find( "storage", "backup_lockfile" );

				int elapsed = time(NULL) - last_checked;
				if ( elapsed < cp_interval )
					sleep( cp_interval - elapsed );
				last_checked = time(NULL);

				LOG_TRACE( "checkpoint begin." );
				StorageEnv::checkpoint();
				LOG_TRACE( "checkpoint end." );

				if( 0 == unlink(backup_lockfile.c_str()) )
				{
					std::string backupdir = conf->find( "storage", "backupdir" );
					LOG_TRACE( "backup begin." );
					StorageEnv::removeoldlogs( );
					StorageEnv::backup( backupdir.c_str() );
					LOG_TRACE( "backup end." );
				}
			}
			catch( ... ) {  }
		}

		return NULL;
	}

};

};

#endif
