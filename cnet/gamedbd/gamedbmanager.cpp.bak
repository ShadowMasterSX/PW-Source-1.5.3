#include "gamedbmanager.h"

#include "conf.h"
#include "storage.h"
#include "dbbuffer.h"
#include "parsestring.h"
#include "conv_charset.h"
#include "localmacro.h"

#include "log.h"

#include "roleid"
#include "crossinfodata"

namespace GNET
{
	void GameDBManager::InitGUID()
	{
		guid = (int64_t)Timer::GetTime() * 0x100000000LL;
		StorageEnv::Storage * plog  = StorageEnv::GetStorage("syslog");
		Marshal::OctetsStream key, value;
		StorageEnv::AtomTransaction txn;
		key << guid;
		printf("Base id =%lld\n",guid);
		while(plog->find(key,value,txn));
		{
			key.clear();
			guid += 0x100000000LL;
			key << guid;
		}
	}

	bool GameDBManager::InitGameDB()
	{
		Conf* conf=Conf::GetInstance();
		zoneid = atoi(conf->find("GameDBServer", "zoneid").c_str());
		areaid = atoi(conf->find("GameDBServer", "aid").c_str());
		
		std::string central_db = conf->find("GameDBServer", "is_central_db");
		if(central_db == "true") SetCentralDB(true);
		std::string recall_server = conf->find("GameDBServer", "is_recall_server");
		if(recall_server == "true") SetRecallServer(true);
		
		if(IsCentralDB() && IsRecallServer())
		{
			Log::log(LOG_ERR, "InitGameDB, Central DB Can not be a Recall Server");
			return false;
		}

		if(conf->find("GameDBServer", "japan_version") == "true") japan_version = true;

		//get deleterole_timeout
		deleterole_timeout = atoi( conf->find("Role","delete_timeout").c_str() );
		Log::log( LOG_INFO, "InitGameDB, Role delete time is %d.\n",deleterole_timeout);

		std::string value = conf->find("OCCUPATION","occupations");
		if (0==value.size())
		{
			Log::log( LOG_ERR, "InitGameDB, no occupations find in configure file." );
			return false;
		}
		std::vector<string> occupation_list;
		if (!ParseStrings(value,occupation_list))
		{
			Log::log( LOG_ERR, "InitGameDB, occupations_list parse failed." );
			return false;
		}
		if (0==occupation_list.size())
		{
			Log::log( LOG_ERR, "InitGameDB, occupations_list parse result is zero." );
			return false;
		}
		
		ReadDBConfig();
		if(IsCentralDB() != (bool)db_config2.is_central_db)
		{
			Log::log(LOG_ERR, "InitGameDB, Can not determine whether it is central DB");
			return false;
		}
		
		std::string accepted_zones = conf->find("GameDBServer", "accepted_zone_list");
		std::string cross_zone_pos = conf->find("GameDBServer", "cross_zone_pos");
		if(IsCentralDB() && !InitAcceptedZoneCrossPos(accepted_zones, cross_zone_pos)) return false;
		
		//get occupation information
		GRoleDetail role;
		struct extend_prop property; 
		for (size_t occp=0;occp<occupation_list.size();occp++)
		{
			role.id=0;	/* decide by Account server */
			role.name=Octets(0);	/* decide by player */
			role.race=atoi(conf->find(occupation_list[occp],"race").c_str());
			role.cls=occp;
			role.gender=0;		/* decide by player */

			role.status.level=1;
			role.status.exp=0;
			role.status.sp=0;
			role.status.pp=0;
			role.status.hp=atoi(conf->find(occupation_list[occp],"hp").c_str());
			role.status.mp=atoi(conf->find(occupation_list[occp],"mp").c_str());
			role.status.posx=atof(conf->find(occupation_list[occp],"posx").c_str());
			role.status.posy=atof(conf->find(occupation_list[occp],"posy").c_str());
			role.status.posz=atof(conf->find(occupation_list[occp],"posz").c_str());
			role.status.worldtag=1;

			property.vitality=atoi(conf->find(occupation_list[occp],"vitality").c_str());
			property.energy=atoi(conf->find(occupation_list[occp],"energy").c_str());
			property.strength=atoi(conf->find(occupation_list[occp],"strength").c_str());
			property.agility=atoi(conf->find(occupation_list[occp],"agility").c_str());
			property.max_hp=atoi(conf->find(occupation_list[occp],"hp").c_str());
			property.max_mp=atoi(conf->find(occupation_list[occp],"mp").c_str());

			property.attack=0;
			property.damage_low=1;
			property.damage_high=1;

			for (int i=0;i<MAGIC_CLASS;i++)
			{
				property.addon_damage[i].damage_low=0;
				property.addon_damage[i].damage_high=0;
			}

			property.damage_magic_low=1;
			property.damage_magic_high=1;
			for (int i=0;i<MAGIC_CLASS;i++)
			{
				property.resistance[i]=0;
			}

			property.defense=1;
			property.armor=0;
			property.max_ap=0;

			role.status.property = Octets(&property,sizeof(property));
		
			// save
			char buffer[128];
			Octets name;
			role.gender = role.cls % 2;
			sprintf( buffer, "cls%dgender%d", role.cls, role.gender );
			name.replace( buffer, strlen(buffer) );
			CharsetConverter::conv_charset_g2u( name, role.name );
			SaveClsDetail( role.cls, role.gender, role );
		}

		//save user table
		SaveUserDefault();
		InitGUID();

		//initial gameregion data
		std::vector<string> gameserver_ids;
		std::vector<string> region_border;
		value = conf->find("GameServerRegion","IDs");
		if (value.empty())
		{
			Log::log( LOG_ERR, "InitGameDB, no GameServerRegion IDs in configure file." );
			return false;
		}
		if (!ParseStrings(value,gameserver_ids))
		{
			Log::log( LOG_ERR, "InitGameDB, no GameServerRegion IDs parse failed." );
			return false;
		}
		region_t region;
		for (size_t i=0;i<gameserver_ids.size();i++)
		{
			region.id=atoi(gameserver_ids[i].c_str());
			value=conf->find("GameServerRegion",gameserver_ids[i]);
			region_border.clear();
			if (!ParseStrings(value,region_border))
			{
				Log::log( LOG_ERR, "InitGameDB, GameServerRegion border parse failed. gs_ids=%s.",
								gameserver_ids[i].c_str() );
				return false;
			}
			region.left=atof(region_border[0].c_str());
			region.right=atof(region_border[1].c_str());
			region.top=atof(region_border[2].c_str());
			region.bottom=atof(region_border[3].c_str());
			Log::log( LOG_INFO, "InitGameDB, acquire region(id=%d),[l=%5.1f,r=%5.1f,t=%5.1f,b=%5.1f].\n",
						region.id,region.left,region.right,region.top,region.bottom);
			gameserver_region.push_back(region);
		}

		return true;
	}

	int GameDBManager::FindGameServer(const point_t& pt)
	{
		Thread::Mutex::Scoped	lock(locker_region);
		for (size_t i=0;i<gameserver_region.size();i++)
		{
			if (gameserver_region[i].IsEnclosed(pt)) return gameserver_region[i].id;
		}
		return -1;
	}

	unsigned int GetDataRoleId(unsigned int cls)
	{
		/*
			武侠-男 16
			法师-女 19
			巫师-男 20
			妖精-女 23
			妖兽-男 24
			刺客-女 27
			羽芒-男 28
			羽灵-女 31
			剑灵-男 18 
			魅灵-女 17
			夜影-男 21
			月仙-女 22
		*/
		switch( cls )
		{
			case 0:		return 16;
			case 1:		return 19;
			case 2:		return 20;
			case 3:		return 23;
			case 4:		return 24;
			case 5:		return 27;
			case 6:		return 28;
			case 7:		return 31;
			case 8:		return 18;
			case 9:		return 17;
			case 10:    return 21;
			case 11:    return 22;			
			default:
				Log::log( LOG_ERR, "GetDataRoleId error cls, cls=%d", cls );
				return 16;
		}
	}

	bool GameDBManager::SaveUserDefault( )
	{
		try
		{
			StorageEnv::Storage * puser = StorageEnv::GetStorage("user");
			StorageEnv::AtomTransaction txn;
			try
			{
				RoleList rolelist;
				rolelist.InitialRoleList();
				rolelist.AddRole( 16 % MAX_ROLE_COUNT );
				rolelist.AddRole( 19 % MAX_ROLE_COUNT );
				rolelist.AddRole( 20 % MAX_ROLE_COUNT );
				rolelist.AddRole( 23 % MAX_ROLE_COUNT );
				rolelist.AddRole( 24 % MAX_ROLE_COUNT );
				rolelist.AddRole( 27 % MAX_ROLE_COUNT );
				rolelist.AddRole( 28 % MAX_ROLE_COUNT );
				rolelist.AddRole( 31 % MAX_ROLE_COUNT );
				rolelist.AddRole( 18 % MAX_ROLE_COUNT );
				rolelist.AddRole( 17 % MAX_ROLE_COUNT );
				rolelist.AddRole( 21 % MAX_ROLE_COUNT );
				rolelist.AddRole( 22 % MAX_ROLE_COUNT );

				User user;
				user.logicuid = 16;
				user.rolelist = rolelist.GetRoleList();
				Marshal::OctetsStream key, value_user;
				key << 16;
				value_user << user;
				puser->insert( key, value_user, txn );
				return true;
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
			Log::log( LOG_ERR, "SaveUserDefault, what=%s\n", e.what() );
		}
		return false;
	}	

	bool GameDBManager::SaveClsDetail(unsigned int cls, char gender, GRoleDetail & role)
	{
		Marshal::OctetsStream key, value, value_base, value_status,
				value_inventory, value_equipment, value_taskinventory, value_storehouse;
		try
		{
			StorageEnv::Storage * pbase = StorageEnv::GetStorage("base");
			StorageEnv::Storage * pstatus = StorageEnv::GetStorage("status");
			StorageEnv::Storage * pinventory = StorageEnv::GetStorage("inventory");
			StorageEnv::Storage * pequipment = StorageEnv::GetStorage("equipment");
			StorageEnv::Storage * pstorehouse = StorageEnv::GetStorage("storehouse");
			StorageEnv::AtomTransaction txn;
			try
			{
				unsigned int roleid = GetDataRoleId(cls);
				key << RoleId( roleid );
				role.id = roleid;

				GRoleBase	base;
				GRoleDetailToBase( role, base );
				base.status = _ROLE_STATUS_NORMAL;
				value_base << base;
				value_status << role.status;
				value_inventory << role.inventory;
				value_equipment << role.equipment;
				value_storehouse << role.storehouse;

				if( !pbase->find( key, value, txn ) )
					pbase->insert( key,value_base, txn );
				if( !pstatus->find( key, value, txn ) )
					pstatus->insert( key, value_status, txn );
				if( !pinventory->find( key, value, txn ) )
					pinventory->insert( key, value_inventory, txn );
				if( !pequipment->find( key, value, txn) )
					pequipment->insert( key, value_equipment, txn );
				if( !pstorehouse->find( key, value, txn ) )
					pstorehouse->insert( key, value_storehouse, txn );
				return true;
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
			Log::log( LOG_ERR, "SaveClsDetail, what=%s\n", e.what() );
		}
		return false;
	}

	bool GameDBManager::GetClsDetail(unsigned int cls, char gender,
					GRoleBase &base, GRoleStatus &status,
					GRolePocket &pocket, GRoleInventoryVector &equipment,
					GRoleStorehouse &storehouse)
	{
		Marshal::OctetsStream key, value_base, value_status, value_inventory, value_equipment,
							value_taskinventory, value_storehouse;
		unsigned int roleid = GetDataRoleId(cls);

		try
		{
			StorageEnv::Storage * pbase = StorageEnv::GetStorage("base");
			StorageEnv::Storage * pstatus = StorageEnv::GetStorage("status");
			StorageEnv::Storage * pinventory = StorageEnv::GetStorage("inventory");
			StorageEnv::Storage * pequipment = StorageEnv::GetStorage("equipment");
			StorageEnv::Storage * pstorehouse = StorageEnv::GetStorage("storehouse");
			StorageEnv::AtomTransaction txn;
			try
			{
				key << RoleId( roleid );

				
				value_base = pbase->find( key, txn );
				value_status = pstatus->find( key, txn );
				value_inventory = pinventory->find( key, txn );
				value_equipment = pequipment->find( key, txn );
				value_storehouse = pstorehouse->find( key, txn );

				value_base >> base;
				value_status >> status;
				value_inventory >> pocket;
				value_equipment >> equipment;
				value_storehouse >> storehouse;

				base.cls = cls;
				base.gender = gender;
				base.help_states.clear();

				status.custom_status.clear();
				status.filter_data.clear();
				status.charactermode.clear();
				status.instancekeylist.clear();
				return true;
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
			Log::log( LOG_ERR, "GetClsDetail, roleid=%d, what=%s\n", roleid, e.what() );
		}
		return false;
	}

	bool GameDBManager::GetClsDetail(unsigned int cls, char gender,
					GRoleBase &base, GRoleStatus &status,
					GRolePocket &pocket, GRoleInventoryVector &equipment,
					GRoleStorehouse &storehouse,
					StorageEnv::Storage * pbase,
					StorageEnv::Storage * pstatus,
					StorageEnv::Storage * pinventory,
					StorageEnv::Storage * pequipment,
					StorageEnv::Storage * pstorehouse,
					StorageEnv::Transaction& txn)
	{
		Marshal::OctetsStream key, value_base, value_status, value_inventory, value_equipment,
							value_taskinventory, value_storehouse;
		unsigned int roleid = GetDataRoleId(cls);

		try
		{
			try
			{
				key << RoleId( roleid );

				
				value_base = pbase->find( key, txn );
				value_status = pstatus->find( key, txn );
				value_inventory = pinventory->find( key, txn );
				value_equipment = pequipment->find( key, txn );
				value_storehouse = pstorehouse->find( key, txn );

				value_base >> base;
				value_status >> status;
				value_inventory >> pocket;
				value_equipment >> equipment;
				value_storehouse >> storehouse;

				base.cls = cls;
				base.gender = gender;
				base.help_states.clear();

				status.custom_status.clear();
				status.filter_data.clear();
				status.charactermode.clear();
				status.instancekeylist.clear();
				return true;
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
			Log::log( LOG_ERR, "GetClsDetail, roleid=%d, what=%s\n", roleid, e.what() );
		}
		return false;
	}

	bool GameDBManager::GetClsPos(unsigned int cls, char gender, float &posx, float &posy, float &posz)
	{
		Marshal::OctetsStream key, value;
		unsigned int roleid = GetDataRoleId(cls);

		try
		{
			StorageEnv::Storage * pstatus = StorageEnv::GetStorage("status");
			StorageEnv::AtomTransaction txn;
			try
			{
				key << RoleId( roleid );

				value = pstatus->find( key, txn );
				GRoleStatus status;
				value >> status;
				posx = status.posx;
				posy = status.posy;
				posz = status.posz;
				return true;
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
			Log::log( LOG_ERR, "GetClsPos, roleid=%d, what=%s\n", roleid, e.what() );
		}
		return false;
	}
	
	void GRoleBaseToDetail( const GRoleBase & base, GRoleDetail & detail )
	{
		detail.id   = base.id;
		detail.name = base.name;
		detail.race = base.race;
		detail.cls  = base.cls;
		detail.gender = base.gender;
		detail.custom_data = base.custom_data;
		detail.custom_stamp= base.custom_stamp;
		detail.spouse  = base.spouse;
		if(base.userid)
			detail.userid = base.userid;
		else
			detail.userid = LOGICUID(base.id);
		detail.create_time = base.create_time;
		detail.lastlogin_time = base.lastlogin_time;

		if(base.cross_data.size() > 0)
		{
			CrossInfoData cds_info;
			Marshal::OctetsStream(base.cross_data) >> cds_info;

			detail.src_zoneid = cds_info.src_zoneid;
		}
	}

	void GRoleDetailToBase( const GRoleDetail & detail, GRoleBase & base )
	{
		base.id   = detail.id;
		base.name = detail.name;
		base.race = detail.race;
		base.cls  = detail.cls;
		base.gender = detail.gender;
		base.custom_data = detail.custom_data;
		base.custom_stamp= detail.custom_stamp;
	}

	void GameDBManager::ReadDBConfig()
	{
		try {
			StorageEnv::CommonTransaction txn;
			Marshal::OctetsStream key, value;
			Marshal::OctetsStream key_dbconfig2, value_dbconfig2;

			key << (int)100;
			key_dbconfig2 << (int)101;
			
			StorageEnv::Storage *pstorage = StorageEnv::GetStorage("config");
			StorageEnv::Storage *pbase = StorageEnv::GetStorage("base");
			try {
				if (pstorage->find(key, value, txn))
					value >> db_config;
				else {
					if(pbase->count()<10000)
					{
						db_config.init_time = time(0);
						db_config.open_time = 0;
					}
					else
					{
						db_config.init_time = 1211353082;
						db_config.open_time = 1211353082;
					}
					pstorage->insert(key, Marshal::OctetsStream() << db_config, txn);
				}

				if (pstorage->find(key_dbconfig2, value_dbconfig2, txn))
				{
					value_dbconfig2 >> db_config2;
				}
				else
				{
					db_config2.is_central_db = IsCentralDB() ? 1 : 0;
					pstorage->insert(key_dbconfig2, Marshal::OctetsStream() << db_config2, txn);
				}
			} catch (DbException &e) {
				throw;
			} catch (...) {
				DbException ee( DB_OLD_VERSION );
				txn.abort( ee );
				throw ee;
			}
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "ReadDBConfig exception, what=%s\n", e.what() );
		}
		LOG_TRACE("ReadDBConfig init_time %d open_time %d is_central_db %d\n", db_config.init_time, db_config.open_time, db_config2.is_central_db);
	}

	void GameDBManager::SaveDBConfig()
	{
		try {
			StorageEnv::CommonTransaction txn;
			Marshal::OctetsStream key, value;
			key << (int)100;
			StorageEnv::Storage *pstorage = StorageEnv::GetStorage("config");
			try {
				pstorage->insert(key, Marshal::OctetsStream() << db_config, txn);
			} catch (DbException &e) {
				throw;
			} catch (...) {
				DbException ee( DB_OLD_VERSION );
				txn.abort( ee );
				throw ee;
			}
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "SaveDBConfig exception, what=%s\n", e.what() );
		}
	}

	int GameDBManager::LoadZoneNameConfig()
	{
		FILE* fp = fopen("serverlist.sev", "rb");
		if (!fp) return -1;

		int ret = 0;
		int zoneid = 0;
		size_t namelen = 0;
		while (fread(&zoneid, sizeof(zoneid), 1, fp) == 1) {
			if (zone_name_map.find(zoneid) != zone_name_map.end()) {
				Log::log(LOG_ERR, "LoadZoneNameConfig: Read duplicated zoneid %d", zoneid);
				ret = -2;
				break;
			}
			if (fread(&namelen, sizeof(namelen), 1, fp) != 1) {
				Log::log(LOG_ERR, "LoadZoneNameConfig: Read name len error");
				ret = -3;
				break;
			}
			if (!namelen || namelen > 32) {
				Log::log(LOG_ERR, "LoadZoneNameConfig: Read name len invalid %d", namelen);
				ret = -4;
				break;
			}
			Octets zone_name;
			zone_name.resize(namelen*2);
			if (fread(zone_name.begin(), namelen*2, 1, fp) != 1) {
				Log::log(LOG_ERR, "LoadZoneNameConfig: Read name error");
				ret = -5;
				break;
			}
			// 强制截断服务器名
			if(zone_name.size() > (unsigned int)zone_name_max_len)
				zone_name.resize(zone_name_max_len);
			bool is_name_duplicated = false;
			for (ZoneNameMap::iterator it = zone_name_map.begin(); it != zone_name_map.end(); ++it) {
				if (it->second == zone_name) {
					is_name_duplicated = true;
					break;
				}
			}
			if (is_name_duplicated) {
				Log::log(LOG_ERR, "LoadZoneNameConfig: Read duplicated name for zoneid %d", zoneid);
				ret = -6;
				break;
			}

			zone_name_map[zoneid] = zone_name;
		}
		fclose(fp);
		return ret;
	}	
	
	bool GameDBManager::InitAcceptedZoneCrossPos(const std::string accepted_zones, const std::string cross_zone_pos)
	{
		std::vector<string> zone_list;
		if(!ParseStrings(accepted_zones, zone_list))
		{
			Log::log( LOG_ERR, "InitGameDB, accepted_zone_list parse failed." );
			return false;
		}

		std::vector<string> zone_pos_list;
		if(!ParseStrings(cross_zone_pos, zone_pos_list))
		{
			Log::log( LOG_ERR, "InitGameDB, cross_zone_pos parse failed." );
			return false;
		}

		if( (zone_list.size() < 2) || (zone_list.size() > 4) )
		{
			Log::log( LOG_ERR, "InitGameDB, zone list count is invalid." );
			return false;
		}

		for(unsigned int i = 0; i < zone_list.size(); ++i)
		{
			int zoneid = atoi(zone_list[i].c_str());
			point_t tmp_point;
			int n = sscanf(zone_pos_list[i].c_str(), "%f:%f:%f", 
				&tmp_point.x, &tmp_point.y, &tmp_point.z);
			if(n != 3) {
				Log::log( LOG_ERR, "InitGameDB, cross_zone_pos parse result error." );
				return false;
			}

			zone_pos_map[zoneid] = tmp_point;
		}

		return true;
	}
};

