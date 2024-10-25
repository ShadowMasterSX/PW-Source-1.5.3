
#ifndef __GNET_DBBUFFER_H
#define __GNET_DBBUFFER_H

#include <string>

#include "thread.h"
#include "marshal.h"
#include "storage.h"
#include "errcode.h"

namespace GNET
{

class DBBuffer
{
public:
	static int buf_find( const char * storage, Octets &key, Octets &value )
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage( storage );
			StorageEnv::AtomTransaction	txn;
			try
			{
				return ( pstorage->find( key, value, txn ) ? 0 : -1 );
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
			return ( DB_NOTFOUND==e.get_errno() ? ERR_DATANOTFIND : e.get_errno() );
		}
	}

	static int buf_insert( const char * storage, Octets &key, Octets &value )
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage( storage );
			StorageEnv::AtomTransaction	txn;
			try
			{
				pstorage->insert( key, value, txn );
				return 0;
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
			return e.get_errno();
		}
	}

	static int buf_insert_nooverwrite( const char * storage, Octets &key, Octets &value )
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage( storage );
			StorageEnv::AtomTransaction	txn;
			try
			{
				pstorage->insert( key, value, txn, DB_NOOVERWRITE );
				return 0;
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
			return e.get_errno();
		}
	}

	static int buf_del( const char * storage, Octets &key )
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage( storage );
			StorageEnv::AtomTransaction	txn;
			try
			{
				pstorage->del( key, txn );
				return 0;
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
			return e.get_errno();
		}
	}

};

};

#endif

