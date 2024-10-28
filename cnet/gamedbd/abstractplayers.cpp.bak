#include "localmacro.h"
#include "saveplayerdata.hrp"
#include "accessdb.h"
#include "grolestatusextraprop"

namespace GNET
{
	
void GenerateCDSRoleName(const Octets & src_name, int src_zoneid, Octets & new_name)
{
	char left_bracket[2] = {'[', '\0'};
	char right_bracket[2] = {']', '\0'};
	Octets zone_name;
	if (!GameDBManager::GetInstance()->GetZoneName(src_zoneid, zone_name)) {
		// 未找到服务器名的服务器，以zoneid代替
		char buf[ZONE_NAME_MAX_LEN_UPLIMIT] = {0};
		snprintf(buf, sizeof(buf), "%d", src_zoneid);
		int len = strlen(buf);
		if(len > ZONE_NAME_MAX_LEN_UPLIMIT/2) len = ZONE_NAME_MAX_LEN_UPLIMIT/2;
		for(int i=ZONE_NAME_MAX_LEN_UPLIMIT-1; i>=0; i--) {
			if(i%2 == 0)
				buf[i] = buf[i/2];
			else
				buf[i] = 0;	
		}
		zone_name.insert(zone_name.end(), buf, len*2);
	}

	new_name = src_name;
	size_t postfix_len = 4 + zone_name.size();
	if (new_name.size() + postfix_len > CDS_MAX_USERNAME_LENGTH) {
		new_name.resize(CDS_MAX_USERNAME_LENGTH - postfix_len);
	}
	new_name.insert(new_name.end(), left_bracket, 2);
	new_name.insert(new_name.end(), zone_name.begin(), zone_name.size());
	new_name.insert(new_name.end(), right_bracket, 2);
}

void GenerateCDSPos(int src_zoneid, float & posx, float & posy, float & posz, int & worldtag)
{
	GameDBManager* gdbm = GameDBManager::GetInstance();

	point_t pos;
	if(gdbm->GetZonePos(src_zoneid, pos))
	{
		posx = pos.x;
		posy = pos.y;
		posz = pos.z;
	}
	else
	{
		Log::log(LOG_ERR, "CrossRelated GenerateCDSPos src_zoneid %d is not accepted!", src_zoneid);

		posx = 0.f;
		posy = 0.f;
		posz = 0.f;
	}
	
	worldtag = 142;
}

/*中央服与普通服角色调转时拷贝的数据*/
void AbstractPlayerData(const GRoleBase & src_base, const GRoleStatus & src_status, GRoleBase & dst_base, GRoleStatus & dst_status)
{
	dst_base.version 		= src_base.version;
	dst_base.race			= src_base.race;
	dst_base.cls			= src_base.cls;
	dst_base.gender			= src_base.gender;
	dst_base.custom_data	= src_base.custom_data;
	dst_base.config_data	= src_base.config_data;
	dst_base.custom_stamp	= src_base.custom_stamp;
	dst_base.delete_time	= src_base.delete_time;
	dst_base.create_time	= src_base.create_time;
	dst_base.lastlogin_time	= src_base.lastlogin_time;
	dst_base.forbid 		= src_base.forbid;
	dst_base.help_states	= src_base.help_states;
	(void)dst_base.reserved2;
	(void)dst_base.reserved3;
	(void)dst_base.reserved4;

	dst_status.version 		= src_status.version;
	dst_status.level 		= src_status.level;
	dst_status.level2 		= src_status.level2;
	dst_status.exp 			= src_status.exp;
	dst_status.sp 			= src_status.sp;
	dst_status.pp 			= src_status.pp;
	dst_status.hp 			= src_status.hp;
	dst_status.mp 			= src_status.mp;
	dst_status.invader_state= src_status.invader_state;
	dst_status.invader_time = src_status.invader_time;
	dst_status.pariah_time 	= src_status.pariah_time;
	dst_status.reputation 	= src_status.reputation;
	dst_status.custom_status= src_status.custom_status; 
	dst_status.filter_data	= src_status.filter_data; 
	dst_status.charactermode= src_status.charactermode; 
	dst_status.instancekeylist	= src_status.instancekeylist; 
	dst_status.dbltime_expire 	= src_status.dbltime_expire;
	dst_status.dbltime_mode 	= src_status.dbltime_mode;
	dst_status.dbltime_begin 	= src_status.dbltime_begin;
	dst_status.dbltime_used 	= src_status.dbltime_used;
	dst_status.dbltime_max 		= src_status.dbltime_max;
	dst_status.time_used 		= src_status.time_used;
	dst_status.dbltime_data		= src_status.dbltime_data; 
	dst_status.storesize 		= src_status.storesize;
	dst_status.petcorral		= src_status.petcorral; 
	dst_status.property			= src_status.property; 
	dst_status.var_data			= src_status.var_data; 
	dst_status.skills			= src_status.skills; 
	dst_status.storehousepasswd	= src_status.storehousepasswd; 
	dst_status.waypointlist		= src_status.waypointlist; 
	dst_status.coolingtime		= src_status.coolingtime; 
	dst_status.npc_relation		= src_status.npc_relation; 
	dst_status.multi_exp_ctrl	= src_status.multi_exp_ctrl; 
	dst_status.storage_task		= src_status.storage_task; 
	dst_status.online_award		= src_status.online_award;
	dst_status.profit_time_data	= src_status.profit_time_data;
	dst_status.king_data 		= src_status.king_data;
	dst_status.meridian_data 	= src_status.meridian_data;
	enum		//原始定义在../gdbclient/db_if.h 请保持一致
	{
		GROLE_STATUS_EXTRAPROP_TOUCH_HALF_TRADE = 0, // Touch
		GROLE_STATUS_EXTRAPROP_DAILY_SIGN_IN,
		GROLE_STATUS_EXTRAPROP_GIFTCARD_HALF_REDEEM, // 礼品码兑换
		GROLE_STATUS_EXTRAPROP_LEADERSHIP,
		GROLE_STATUS_EXTRAPROP_GENERALCARD_COLLECTION,
		GROLE_STATUS_EXTRAPROP_FATERING,
		GROLE_STATUS_EXTRAPROP_CLOCK_DATA,
		GROLE_STATUS_EXTRAPROP_RAND_MALL_DATA,
		GROLE_STATUS_EXTRAPROP_WORLD_CONTRIBUTION,
		GROLE_STATUS_EXTRAPROP_ASTROLABE_EXTERN,

		GROLE_STATUS_EXTRAPROP_COUNT, // 总数，请放最后
	};
	GRoleStatusExtraProp dst_extraprop;
	if(dst_status.extraprop.size())
	{
		try{ 
			Marshal::OctetsStream(dst_status.extraprop) >> dst_extraprop; 
		}catch(Marshal::Exception){ 
			dst_extraprop.data.clear(); 
		}
	}
	if(src_status.extraprop.size())
	{
		GRoleStatusExtraProp src_extraprop;
		try{
			Marshal::OctetsStream(src_status.extraprop) >> src_extraprop;
		}catch(Marshal::Exception){
			src_extraprop.data.clear();
		}
		for(std::map<int,Octets>::iterator it=src_extraprop.data.begin(); it!=src_extraprop.data.end(); ++it)
		{
			switch(it->first)
			{
				case GROLE_STATUS_EXTRAPROP_DAILY_SIGN_IN:	//每日签到
				case GROLE_STATUS_EXTRAPROP_LEADERSHIP:
				case GROLE_STATUS_EXTRAPROP_GENERALCARD_COLLECTION:
				case GROLE_STATUS_EXTRAPROP_FATERING:
				case GROLE_STATUS_EXTRAPROP_CLOCK_DATA:
				case GROLE_STATUS_EXTRAPROP_ASTROLABE_EXTERN:	
					dst_extraprop.data[it->first] = it->second;
					break;
				
				case GROLE_STATUS_EXTRAPROP_TOUCH_HALF_TRADE:
				case GROLE_STATUS_EXTRAPROP_GIFTCARD_HALF_REDEEM:
				case GROLE_STATUS_EXTRAPROP_RAND_MALL_DATA:	
				case GROLE_STATUS_EXTRAPROP_WORLD_CONTRIBUTION:	
					break;

				default:
					break;
			}
		}
	}
	dst_status.extraprop = Marshal::OctetsStream() << dst_extraprop;	
	dst_status.title_data		= src_status.title_data;
	dst_status.reincarnation_data	= src_status.reincarnation_data;
	dst_status.realm_data = src_status.realm_data;
	(void)dst_status.reserved2;
	(void)dst_status.reserved3;
}

/*中央服普通服通用. 注意:此函数会改变输入的base equipment*/
void BuildRoleInfo(GRoleBase & base, GRoleStatus & status, GRoleEquipment & equipment, User & user, GRoleInfo & roleinfo)
{
	roleinfo.id 			= base.id;
	roleinfo.name			= base.name;
	roleinfo.race 			= base.race;
	roleinfo.cls  			= base.cls;
	roleinfo.gender 		= base.gender;
	roleinfo.level 			= status.level;
	roleinfo.level2 		= status.level2;
	roleinfo.posx 			= status.posx;
	roleinfo.posy 			= status.posy;
	roleinfo.posz 			= status.posz;
	roleinfo.worldtag 		= status.worldtag;
	roleinfo.custom_data	= base.custom_data; 
	roleinfo.custom_stamp 	= base.custom_stamp;
	roleinfo.custom_status 	= status.custom_status;
	roleinfo.charactermode 	= status.charactermode;
	roleinfo.equipment.swap(equipment.inv);
	roleinfo.status 		= base.status;
	roleinfo.delete_time 	= base.delete_time;
	roleinfo.create_time 	= base.create_time;
	roleinfo.lastlogin_time = base.lastlogin_time;
	roleinfo.forbid.swap(base.forbid);
	roleinfo.referrer_role	= 0;
	roleinfo.cash_add		= 0;
	if (base.cross_data.size())
		Marshal::OctetsStream(base.cross_data) >> roleinfo.cross_data;
	roleinfo.cash_add = user.cash_add;
	if (user.reference.size())
	{
		GRefStore ref_store;
		Marshal::OctetsStream(user.reference) >> ref_store;
		roleinfo.referrer_role = ref_store.referrer_roleid;
	}
	roleinfo.reincarnation_data = status.reincarnation_data;
	roleinfo.realm_data = status.realm_data;
}

/**
 * 从跨服抽取角色数据到原服前的数据库类型检查
 * @param srcpath 跨服数据的路径
 * @return true 类型检查正确 false 类型检查错误
 */
bool CheckDBCrossType(const char* srcpath)
{
	if(GetDBCrossType() != 0)
	{
		printf("AbstractPlayers error: Dest DB type invalid\n");
		return false;
	}

	if(GetStandaloneDBCrossType(srcpath) != 1)
	{
		printf("AbstractPlayers error: Src DB type invalid\n");
		return false;
	}

	return true;
}

struct CrossData
{
	int roleid;
	int remote_roleid;
	int data_timestamp;

	bool operator < (const CrossData& rhs) const
	{
		return roleid < rhs.roleid;
	}
};

class FetchCrossLockedRoleQuery: public StorageEnv::IQuery
{
public:
	std::set<CrossData> cross_locked_roles;

	bool Update(StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try {
			Marshal::OctetsStream os_key(key), os_value(value);
			
			int id = -1;
			os_key >> id;
			
			GRoleBase base;
			os_value >> base;
			
			CrossInfoData info;
			if(base.cross_data.size() > 0) {
				Marshal::OctetsStream(base.cross_data) >> info;
			}
			
			if(base.status == _ROLE_STATUS_CROSS_LOCKED) {
				CrossData data;
				data.roleid = id;
				data.remote_roleid = info.remote_roleid;
				data.data_timestamp = info.data_timestamp;
				
				cross_locked_roles.insert(data);
			}
		} catch(Marshal::Exception e) {
			Log::log( LOG_ERR, "FetchCrossLockedRoleQuery, exception\n");
		}
		
		return true;
	}
};

static bool find(DBStandalone* pdb, StorageEnv::Uncompressor* uncomp, const Octets& key, Octets& value)
{
	size_t value_len;
	if(void* val = pdb->find(key.begin(), key.size(), &value_len)) {
		uncomp->Update(Octets(val, value_len)).swap(value);
		free(val);
		
		return true;
	}
	
	return false;
}

static bool put(DBStandalone* pdb, StorageEnv::Compressor* comp, const Octets& key, const Octets& value)
{
	if(key.size() == 0) {
		Log::log(LOG_ERR, "put key.size 0");
		return false;
	}
	
	Octets com_val = comp->Update(value);
	pdb->put(key.begin(), key.size(), com_val.begin(), com_val.size());
	
	return true;
}

/**
 * 将原服数据库中CROSS_LOCKED状态的玩家数据从跨服数据库中抽取回来，命令行参数：
 * 		./gamedbd.wdb gamesys.conf abstractroles centraldb/dbdata xx(服务器的zoneid)
 * 此操作除了修改原服数据库，同时也会修改跨服数据库，将相应玩家的状态设置为CROSS_LOCKED，因此:
 * 若只是单纯的抽取数据操作，必须将修改后的跨服数据库拷贝回原目录。若多个服务器(甲乙丙)都要求抽取数据，必须按顺序依次进行
 * 若是合服操作，则不必将修改后的跨服数据库拷贝回原目录。因为跨服会删除相应zoneid的所有角色
 *
 * @param srcpath 跨服的数据路径
 * @param zoneid 原服的zoneid
 */
void AbstractPlayers(const char* srcpath, int zoneid)
{
	std::string src_dir = srcpath;
	if(!CheckDBCrossType(srcpath)) { //原服和跨服的数据库类型检查
		Log::log(LOG_ERR, "AbstractPlayers Check DB type failed");
		return;
	}
	
	Log::log(LOG_INFO, "FetchCrossLockedRoleQuery begin...");
	
	//查找原服上处于CROSS_LOCKED状态的角色列表
	FetchCrossLockedRoleQuery q;
	try {
		StorageEnv::Storage* pstorage = StorageEnv::GetStorage("base");
		StorageEnv::AtomTransaction	txn;
		
		try {
			StorageEnv::Storage::Cursor cursor = pstorage->cursor(txn);
			cursor.walk( q );
		} catch( DbException e ) { 
			throw; 
		} catch( ... ) {
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
	} catch( DbException e ) {
		Log::log( LOG_ERR, "AbstractPlayers, error when walk for FetchCrossLockedRoleQuery, what=%s\n", e.what() );
	}
	
	Log::log(LOG_INFO, "FetchCrossLockedRoleQuery end, locked set size %d", q.cross_locked_roles.size());

	StorageEnv::Uncompressor* uncompressor = new StorageEnv::Uncompressor();
	StorageEnv::Compressor* compressor = new StorageEnv::Compressor();

	DBStandalone* pbase_alone = new DBStandalone( (src_dir + "/base").c_str() );
	DBStandalone* pstatus_alone = new DBStandalone( (src_dir + "/status").c_str() );
	DBStandalone* pinventory_alone = new DBStandalone( (src_dir + "/inventory").c_str() );
	DBStandalone* pequipment_alone = new DBStandalone( (src_dir + "/equipment").c_str() );
	DBStandalone* pstore_alone = new DBStandalone( (src_dir + "/storehouse").c_str() );
	DBStandalone* ptask_alone = new DBStandalone( (src_dir + "/task").c_str() );

	pbase_alone->init();
	pstatus_alone->init();
	pinventory_alone->init();
	pstore_alone->init();
	pequipment_alone->init();
	ptask_alone->init();

	int ignore_count = 0;
	int fix_count = 0;
	int process_count = 0;
	bool finished = false;

	std::set<CrossData>::const_iterator it = q.cross_locked_roles.begin();
	while(!finished) {
		try {
			StorageEnv::Storage* pbase = StorageEnv::GetStorage("base");
			StorageEnv::Storage* pstatus = StorageEnv::GetStorage("status");
			StorageEnv::Storage* pinventory = StorageEnv::GetStorage("inventory");
			StorageEnv::Storage* pequipment = StorageEnv::GetStorage("equipment");
			StorageEnv::Storage* pstorehouse = StorageEnv::GetStorage("storehouse");
			StorageEnv::Storage* ptask = StorageEnv::GetStorage("task");
			StorageEnv::CommonTransaction txn;
			
			int i = 0;
			for(; it != q.cross_locked_roles.end() && i < CHECKPOINT_THRESHOLD; ++it, ++i) {
				try {
					Marshal::OctetsStream os_key, central_os_key, os_base, os_status, os_pocket, os_equip, os_store, os_task;
					
					const CrossData& data = *it;
					int central_roleid = data.remote_roleid; //原服上的remote_roleid，正是跨服上的roleid
 
					os_key << data.roleid;
					central_os_key << central_roleid;
					bool src_valid = false;
					
					GRoleBase ds_base, central_base;
					GRoleStatus ds_status, central_status;
					GRolePocket central_pocket;
					GRoleEquipment central_equip;
					GRoleStorehouse central_store;
					GRoleTask central_task;

					Marshal::OctetsStream(pbase->find(os_key, txn)) >> ds_base; //原服base
					Marshal::OctetsStream(pstatus->find(os_key, txn)) >> ds_status; //原服status

					bool need_fix = false;
					if(!find(pbase_alone, uncompressor, central_os_key, os_base)) {
						Log::log(LOG_ERR, "Can not find %d in centraldb->base, need fix", central_roleid);
						need_fix = true;
					} else {
						os_base >> central_base; //跨服base	
						CrossInfoData central_cross_info;
						Marshal::OctetsStream(central_base.cross_data) >> central_cross_info; //跨服上的cross_info

						if(central_cross_info.src_zoneid != zoneid || central_cross_info.remote_roleid != data.roleid) {
							Log::log(LOG_ERR, "role %d src_zoneid not match %d:%d, or remote_roleid not match %d:%d, need fix", central_roleid, central_cross_info.src_zoneid, zoneid, central_roleid, data.roleid);
							need_fix = true;
						} else if(central_cross_info.data_timestamp <= data.data_timestamp) {
							Log::log(LOG_ERR, "role %d data_timestamp invalid %d:%d, need fix", central_roleid, central_cross_info.data_timestamp, data.data_timestamp);
							need_fix = true;
						} else {
							src_valid = true;
						}
					}
										
					if(!find(pstatus_alone, uncompressor, central_os_key, os_status)) {
						Log::log(LOG_ERR, "Can not find %d in centraldb->status, need fix", central_roleid);
						need_fix = true;
					} else {
						os_status >> central_status; //跨服status
					}
					
					if(!find(pinventory_alone, uncompressor, central_os_key, os_pocket)) {
						Log::log(LOG_ERR, "Can not find %d in centraldb->inventory, need fix", central_roleid);
						need_fix = true;
					} else {
						os_pocket >> central_pocket; //跨服的inventory
					}
						
					if(!find(pstore_alone, uncompressor, central_os_key, os_store)) {
						Log::log(LOG_ERR, "Can not find %d in centraldb->storehouse, need fix", central_roleid);
						need_fix = true;
					} else {
						os_store >> central_store; //跨服的storehouse
					}
					
					if(!find(pequipment_alone, uncompressor, central_os_key, os_equip)) {
						Log::log(LOG_ERR, "Can not find %d in centraldb->equipment, need fix", central_roleid);
						need_fix = true;
					} else {
						os_equip >> central_equip; //跨服的equipment
					}

					if(!find(ptask_alone, uncompressor, central_os_key, os_task)) {
						Log::log(LOG_ERR, "Can not find %d in centraldb->task, need fix", central_roleid);
						need_fix = true;
					} else {
						os_task >> central_task; //跨服的task
					}
					
					if(need_fix) {
						++fix_count;
					} else {
						++process_count;
						//将跨服中变化的数据拷贝回原服
						AbstractPlayerData(central_base, central_status, ds_base, ds_status);
					}
					
					ds_base.status = _ROLE_STATUS_NORMAL;
					//将更新后的base表写回原服数据库
					pbase->insert(os_key, Marshal::OctetsStream() << ds_base, txn);
					
					if(!need_fix) {
						//将更新后的status表写回原服数据库
						pstatus->insert(os_key, Marshal::OctetsStream() << ds_status, txn);
						//inventory, equipment, storehouse, task表在跨服和回跨过程中都是完全覆盖，故将跨服最新的数据写回原服就可以了
						pinventory->insert(os_key, Marshal::OctetsStream() << central_pocket, txn);
						pequipment->insert(os_key, Marshal::OctetsStream() << central_equip, txn);
						pstorehouse->insert(os_key, Marshal::OctetsStream() << central_store, txn);
						ptask->insert(os_key, Marshal::OctetsStream() << central_task, txn);
					}
					
					if(src_valid) {
						//将跨服库中玩家状态置为CROSS_LOCKED
						central_base.status = _ROLE_STATUS_CROSS_LOCKED;
						put(pbase_alone, compressor, central_os_key, Marshal::OctetsStream() << central_base);
					}
					
					//Log::log(LOG_INFO, "Process roleid %d, central_roleid %d", it->roleid, central_roleid);
				} catch(DbException e) { 
					Log::log(LOG_ERR, "AbstractPlayer, exception what=%s roleid %d", e.what(), it->roleid);
					++ignore_count;
				} catch(...) {
					Log::log(LOG_ERR, "Data Error roleid %d", it->roleid);
					++ignore_count;
				}
			} 		

			if(it == q.cross_locked_roles.end()) finished = true;
		} catch(DbException e) {
			Log::log(LOG_ERR, "AbstractPlayer, exception what=%s roleid %d", e.what(), it->roleid);
			++ignore_count;
		}
		
		StorageEnv::checkpoint();
		pbase_alone->checkpoint();
		pstatus_alone->checkpoint();	
		pinventory_alone->checkpoint();
		pstore_alone->checkpoint();
		pequipment_alone->checkpoint();
		ptask_alone->checkpoint();

		Log::log(LOG_INFO, "checkpoint ignore %d records, fix %d records, process %d records",
			ignore_count, fix_count, process_count);
	}

	pbase_alone->checkpoint();
	pstatus_alone->checkpoint();	
	pinventory_alone->checkpoint();
	pstore_alone->checkpoint();
	pequipment_alone->checkpoint();
	ptask_alone->checkpoint();

	delete pbase_alone;
	delete pstatus_alone;
	delete pinventory_alone;
	delete pstore_alone;
	delete pequipment_alone;
	delete ptask_alone;

	delete uncompressor;
	delete compressor;

	Log::log(LOG_INFO, "Abstract roles end. ignore %d records, fix %d records, process %d records",
			ignore_count, fix_count, process_count);
}

class FetchZonePlayersQuery: public StorageEnv::IQuery
{
public:
	int zoneid;
	std::map< int/*roleid*/, std::pair<int/*userid*/, Octets> > zone_players;
	
	FetchZonePlayersQuery(int _zoneid): zoneid(_zoneid){ zone_players.clear(); }

	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try 
		{
			Marshal::OctetsStream os_key(key), os_value(value);
			
			int roleid = -1;
			os_key >> roleid;
			
			GRoleBase base;
			os_value >> base;
			
			CrossInfoData info;
			if(base.cross_data.size() > 0) 
			{
				Marshal::OctetsStream(base.cross_data) >> info;
			}
			
			if(info.src_zoneid == zoneid)
			{
				int userid = (base.userid == 0 ? LOGICUID(roleid) : base.userid);
				std::pair<int, Octets> val;
				val.first = userid;
				val.second = base.name;
				
				zone_players.insert(std::make_pair(roleid, val));
			}
		} 
		catch ( Marshal::Exception e ) 
		{
			Log::log( LOG_ERR, "FetchZonePlayersQuery, exception\n" );
		}
		
		return true;
	}
};

/**
 * 删除跨服服务器上，来自zoneid的角色
 * @param zoneid 角色的原服zoneid
 */
void DelCentralZonePlayers(int zoneid)
{
	if(GetDBCrossType() != 1) //必须是跨服
	{
		Log::log(LOG_ERR, "Invalid DB cross type");	
		return;
	}
		
	if(zoneid <= 0)
	{
		Log::log(LOG_ERR, "Invalid parameter zoneid %d", zoneid);
		return;
	}
	
	LOG_TRACE("players from zone %d will be deleted from central DB", zoneid);
	FetchZonePlayersQuery q(zoneid);
	
	try
	{
		StorageEnv::Storage* pstorage = StorageEnv::GetStorage("base");
		StorageEnv::AtomTransaction txn;
		
		try
		{
			StorageEnv::Storage::Cursor cursor = pstorage->cursor(txn);
			cursor.walk( q );
		}
		catch( DbException e ) { throw; }
		catch( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "DelCentralZonePlayers, error when walk for FetchZonePlayersQuery, what=%s\n", e.what() );
	}
	LOG_TRACE("FetchZonePlayersQuery end, total player count is %d", q.zone_players.size()); 
	
	std::map< int, std::pair<int, Octets> >::const_iterator it = q.zone_players.begin();
	bool finished = false;
	int del_success_count = 0;
	
	while(!finished)
	{
		try
		{
			StorageEnv::Storage* puser = StorageEnv::GetStorage("user");
			StorageEnv::Storage* pbase = StorageEnv::GetStorage("base");
			StorageEnv::Storage* pstatus = StorageEnv::GetStorage("status");
			StorageEnv::Storage* pinventory = StorageEnv::GetStorage("inventory");
			StorageEnv::Storage* pequipment = StorageEnv::GetStorage("equipment");
			StorageEnv::Storage* pstorehouse = StorageEnv::GetStorage("storehouse");
			StorageEnv::Storage* ptask = StorageEnv::GetStorage("task");
			StorageEnv::Storage* prolename = StorageEnv::GetStorage("rolename");
			StorageEnv::CommonTransaction txn;
			
			int i = 0;
			for(; it != q.zone_players.end() && i < CHECKPOINT_THRESHOLD; ++it, ++i)
			{
				int roleid = it->first;
				const std::pair<int, Octets>& val = it->second;
				int userid = val.first;
				const Octets& name = val.second;

				try
				{
					Marshal::OctetsStream os_key;
					os_key << roleid;

					pbase->del(os_key, txn);
					pstatus->del(os_key, txn);
					pinventory->del(os_key, txn);
					pequipment->del(os_key, txn);
					pstorehouse->del(os_key, txn);
					ptask->del(os_key, txn);
					prolename->del(name, txn);
					
					Marshal::OctetsStream user_key, os_user;
					user_key << userid;
					if(puser->find(user_key, os_user, txn))
					{
						User user;
						os_user >> user;
						if(user.logicuid == (unsigned int)LOGICUID(roleid))
						{
							RoleList t(user.rolelist);
							t.DelRole(roleid);
							user.rolelist = t.GetRoleList();
							puser->insert(user_key, Marshal::OctetsStream() << user, txn);
						}
						else
						{
							Log::log(LOG_INFO, "INFO:logicuid %d of user %d does not match roleid %d", user.logicuid, userid, roleid);
						}
					}
					else
					{
						Log::log(LOG_ERR, "Can not find userid %d for role %d", userid, roleid);
					}
					
					++del_success_count; 
				}
				catch(DbException e) 
				{  
					Log::log(LOG_ERR, "DelCentralZonePlayers, exception what=%s roleid %d", e.what(), roleid);
				}
				catch(...)
				{
					Log::log(LOG_ERR, "Data Error roleid %d", roleid);
				}
			}

			if(it == q.zone_players.end()) finished = true;
		}
		catch(DbException e)
		{
			Log::log(LOG_ERR, "DelCentralZonePlayers, exception what=%s roleid %d", e.what(), it->first);
		}
		catch(...)
		{
			Log::log(LOG_ERR, "DelCentralZonePlayers Error roleid %d", it->first);
		}

		StorageEnv::checkpoint();
		LOG_TRACE("checkpoint delete %d success", del_success_count);
	}
	
	LOG_TRACE("DelCentralZonePlayers end, delete %d players successfully", del_success_count);
}

}
