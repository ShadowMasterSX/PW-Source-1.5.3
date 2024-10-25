#ifndef __GNET_CLSCONFIG_H
#define __GNET_CLSCONFIG_H

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "log.h"

#include "dbbuffer.h"

#include "roleid"
#include "grolebase"
#include "grolestatus"
#include "groleinventory"
#include "grolestorehouse"
#include "groledetail"
#include "groletableclsconfig"

namespace GNET
{
void ImportClsConfig()
{
	std::string cls_conf = "./clsconfig";
	
	Conf *conf = Conf::GetInstance();
	std::string recall_server = conf->find("GameDBServer", "is_recall_server");
	if(recall_server == "true") cls_conf = "./clsconfig_80";

	if( 0 != access(cls_conf.c_str(), R_OK) ) return;
	
	std::string datadir = StorageEnv::get_datadir();
	StorageEnv::checkpoint();
	StorageEnv::Close();

	std::string clsconfigfile = datadir + "/clsconfig";
	system( ("/bin/cp -f " + cls_conf + " " + clsconfigfile).c_str() );

	StorageEnv::Open();

	if( 0 != access(clsconfigfile.c_str(),R_OK) )
		return;

	unsigned int roleid;
	for( roleid = 16; roleid<32; roleid ++ )
	{

		try
		{
			StorageEnv::Storage * pclsconfig = StorageEnv::GetStorage("clsconfig");
			StorageEnv::Storage * pbase = StorageEnv::GetStorage("base");
			StorageEnv::Storage * pstatus = StorageEnv::GetStorage("status");
			StorageEnv::Storage * pinv = StorageEnv::GetStorage("inventory");
			StorageEnv::Storage * pequip = StorageEnv::GetStorage("equipment");
			StorageEnv::Storage * pstore= StorageEnv::GetStorage("storehouse");
			pclsconfig->SetCompressor( new StorageEnv::NullCoder(),new StorageEnv::NullCoder());
			StorageEnv::CommonTransaction txn;
			try
			{
				GRoleTableClsconfig	clsconfig;
				Marshal::OctetsStream key_temp;
				key_temp << roleid;
				Marshal::OctetsStream(pclsconfig->find( key_temp, txn )) >> clsconfig;

				pbase->insert(key_temp,Marshal::OctetsStream()<<clsconfig.base,txn );
				pstatus->insert(key_temp,Marshal::OctetsStream()<<clsconfig.status,txn );
				pinv->insert(key_temp,Marshal::OctetsStream()<<clsconfig.inventory,txn);
				pequip->insert(key_temp,Marshal::OctetsStream()<<clsconfig.equipment,txn);
				pstore->insert(key_temp,Marshal::OctetsStream()<<clsconfig.storehouse,txn );
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
			Log::log( LOG_ERR, "ImportClsConfig, roleid=%d, what=%s\n", roleid, e.what() );
		}
	}

	StorageEnv::checkpoint();
	StorageEnv::Close();
	StorageEnv::Open();
	Log::log( LOG_INFO, "ClsConfig Imported ." );
}
void ExportClsConfig( )
{
	std::string datadir = StorageEnv::get_datadir();
	std::string clsconfigfile = datadir + "/clsconfig";

	unsigned int roleid;
	for( roleid = 16; roleid<32; roleid ++ )
	{
		Marshal::OctetsStream key, value_all, value_base, value_status,
				value_inventory, value_equipment, value_task, value_storehouse;
		GRoleTableClsconfig	clsconfig;

		try
		{
			StorageEnv::Storage * pclsconfig = StorageEnv::GetStorage("clsconfig");
			StorageEnv::Storage * pbase = StorageEnv::GetStorage("base");
			StorageEnv::Storage * pstatus = StorageEnv::GetStorage("status");
			StorageEnv::Storage * pinventory = StorageEnv::GetStorage("inventory");
			StorageEnv::Storage * pequipment = StorageEnv::GetStorage("equipment");
			StorageEnv::Storage * pstorehouse = StorageEnv::GetStorage("storehouse");
			pclsconfig->SetCompressor( new StorageEnv::NullCoder(), new StorageEnv::NullCoder() );
			StorageEnv::CommonTransaction txn;
			try
			{
				key << RoleId(roleid);
				if( pbase->find( key, value_base, txn ) )
					value_base >> clsconfig.base;

				if( pstatus->find( key, value_status, txn ) )
					value_status >> clsconfig.status;

				if( pinventory->find( key, value_inventory, txn ) )
					value_inventory >> clsconfig.inventory;

				if( pequipment->find( key, value_equipment, txn ) )
					value_equipment >> clsconfig.equipment;

				if( pstorehouse->find( key, value_storehouse, txn ) )
					value_storehouse >> clsconfig.storehouse;

				value_all << clsconfig;
				pclsconfig->insert( key, value_all, txn );

				printf("exportclsconfig roleid=%d\n",roleid);
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
			Log::log( LOG_ERR, "SaveClsConfig, roleid=%d, what=%s\n", roleid, e.what() );
		}
	}

	StorageEnv::checkpoint();
	StorageEnv::Close();
	
	std::string cls_conf = "./clsconfig";
	Conf *conf = Conf::GetInstance();
	std::string recall_server = conf->find("GameDBServer", "is_recall_server");
	if(recall_server == "true") cls_conf = "./clsconfig_80";

	system( ("/bin/cp -f " + clsconfigfile + " " + cls_conf).c_str() );
	Log::log( LOG_INFO, "ClsConfig Exported ." );
	
	StorageEnv::Open();
}

void ClearClsConfig( )
{
	unsigned int roleid;
	for( roleid = 16; roleid<32; roleid ++ )
	{
		Marshal::OctetsStream key;
		key << RoleId(roleid);

		try
		{
			StorageEnv::Storage * pbase = StorageEnv::GetStorage("base");
			StorageEnv::Storage * pstatus = StorageEnv::GetStorage("status");
			StorageEnv::Storage * pinventory = StorageEnv::GetStorage("inventory");
			StorageEnv::Storage * pequipment = StorageEnv::GetStorage("equipment");
			StorageEnv::Storage * pstorehouse = StorageEnv::GetStorage("storehouse");
			StorageEnv::CommonTransaction txn;

			pbase->del( key, txn );
			pstatus->del( key, txn );
			pinventory->del( key, txn );
			pequipment->del( key, txn );
			pstorehouse->del( key, txn );
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "ClearClsConfig, roleid=%d, what=%s\n", roleid, e.what() );
		}
	}

	StorageEnv::checkpoint();
}

void ClearClsWayPoint()
{
	unsigned int roleid;
	for( roleid = 16; roleid<32; roleid ++ )
	{
		Marshal::OctetsStream key;
		key << RoleId(roleid);

		try
		{
			StorageEnv::Storage * pstatus = StorageEnv::GetStorage("status");
			StorageEnv::CommonTransaction txn;

			GRoleStatus rs;
			Marshal::OctetsStream(pstatus->find(key,txn)) >> rs;
			rs.waypointlist.clear();
			pstatus->insert( key, Marshal::OctetsStream() << rs, txn );
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "ClearClsWayPoint, roleid=%d, what=%s\n", roleid, e.what() );
		}
	}

	StorageEnv::checkpoint();
}

}

#endif

