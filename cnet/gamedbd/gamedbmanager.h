#ifndef __GNET_GAMEDBMANAGER_H
#define __GNET_GAMEDBMANAGER_H
/*		this class provide management tools for gamedb, such as:
 *	   	1. initialize tab and data
 *		2. query data
 *		3. show statistic informations
 */

#include <vector>
#include <map>

#include "thread.h"
#include "conv_charset.h"
#include "statistic.h"

#include "groleforbid"
#include "grolebase"
#include "grolestatus"
#include "groleinventory"
#include "gshoplog"
#include "groledetail"
#include "roleid"
#include "stocklog"
#include "user"
#include "userid"
#include "dbconfig"
#include "map.h"
#include <storage.h>
#include "dbconfig2"

#define MAGIC_CLASS		5
#define ROLELIST_DEFAULT	0x80000000
#define MAX_ROLE_COUNT		16
#define UNAUTHORIZED_SERVER	0x90
#define CDS_MAX_USERNAME_LENGTH (40-2)
#define ZONE_NAME_MAX_LEN_DEFAULT 	4
#define ZONE_NAME_MAX_LEN_UPLIMIT	16

#define ROLE_NAME_STAT_USED	1	//名字正在被使用
#define ROLE_NAME_STAT_OBSOLETE 2	//名字已经被废弃掉

namespace GNET
{
	struct extend_prop
	{
		/* 基础属性 */
		int vitality;           //命
		int energy;             //神
		int strength;           //力
		int agility;            //敏
		int max_hp;             //最大hp
		int max_mp;             //最大mp
		int hp_gen;             //hp恢复速度
		int mp_gen;             //mp恢复速度 

		/* 运动速度*/ 
		float walk_speed;       //行走速度 单位  m/s
		float run_speed;        //奔跑速度 单位  m/s
		float swim_speed;       //游泳速度 单位  m/s
		float flight_speed;     //飞行速度 单位  m/s

		/* 攻击属性*/
		int attack;             //攻击率 attack rate
		int damage_low;         //最低damage
		int damage_high;        //最大物理damage
		int attack_speed;       //攻击时间间隔 以tick为单位
		float attack_range;     //攻击范围
		struct                  //物理攻击附加魔法伤害
		{       
			int damage_low;
			int damage_high;
		} addon_damage[MAGIC_CLASS];

		/* 魔法相关 */
		int damage_magic_low;           //魔法最低攻击力
		int damage_magic_high;          //魔法最高攻击力
		int resistance[MAGIC_CLASS];    //魔法抗性

		/* 防御属性 */  
		int defense;            //防御力
		int armor;              //闪躲率（装甲等级）
		int max_ap;				//最大怒气值
	};
	struct point_t
	{
		float x;
		float y;
		float z;
		point_t() { }
		point_t(float _x,float _y,float _z) : x(_x),y(_y),z(_z) { }
	};
	struct region_t
	{
		int		id;
		float	left;
		float	right;
		float	top;
		float 	bottom;
		region_t() { }
		region_t(char _id,float _l,float _r,float _t,float _b) : id(_id),left(_l),right(_r),top(_t),bottom(_b) { }	
		bool IsEnclosed(point_t pt) { return (pt.x>left && pt.x<right) && (pt.z>bottom && pt.z<top ); }
	};
	class RoleList
	{
		unsigned int rolelist;
		int count;
		int cur_role;
	public:
		RoleList() : rolelist(0),count(0),cur_role(0) { }
		RoleList(unsigned int r) : rolelist(r),count(0),cur_role(0) { }
		~RoleList() { }
		void operator=(const RoleList& rhs) { rolelist=rhs.rolelist; }
		bool IsRoleListInitialed()
		{
			return (rolelist & ROLELIST_DEFAULT) != 0;
		}
		bool IsRoleExist(int roleid)
		{
			return (rolelist & (1<<(roleid % MAX_ROLE_COUNT))) != 0;
		}
		void InitialRoleList()
		{
			rolelist = ROLELIST_DEFAULT;
		}
		unsigned int GetRoleList() 
		{
			return rolelist;
		}
		int GetRoleCount()
		{
			if (!IsRoleListInitialed()) return -1;  //rolelist is not initialized
			count=0;
			for (int id=0;id<MAX_ROLE_COUNT;id++)
			{
				if (IsRoleExist(id)) count++;
			}
			return count;
		}
		int AddRole()
		{
			if (!IsRoleListInitialed()) { return -1; }
			if (GetRoleCount()==MAX_ROLE_COUNT) { return -1; }
			int id=0;
			for (;id<MAX_ROLE_COUNT && IsRoleExist(id);id++);
			rolelist +=(1<<id);
			return id;	
		}		
		int AddRole(int roleid)
		{
			if (!IsRoleListInitialed()) { return -1; }
			if (IsRoleExist(roleid)) { return roleid; } //the role will be overlayed
			if (GetRoleCount()==MAX_ROLE_COUNT) { return -1; }
			if (roleid<0 || roleid >MAX_ROLE_COUNT-1) { return -1;}
			rolelist +=(1<<roleid);  
			return roleid;
		}
		bool DelRole(int roleid)
		{
			if (!IsRoleListInitialed()) return false;
			if (!IsRoleExist(roleid)) return false;
			return (rolelist -= 1<<(roleid % MAX_ROLE_COUNT)) != 0;
		}
		int GetNextRole()
		{
			while (cur_role<MAX_ROLE_COUNT)
				if (IsRoleExist(cur_role++)) return cur_role-1;
			return -1;
		}
		void SeekToBegin()
		{
			cur_role=0;
		}
	};

	class DestroyProgram
	{
		bool start;
		size_t counter;
	public:
		DestroyProgram():start(false),counter(0){}
		void SetStart(bool b){ start = b; }
		bool IsStart(){ return start; }
		void IncCounter(){ ++counter; }
		size_t GetCounter(){ return counter; }
	};

	#define	OBJECT_COUNT	1024
	#define	MATERIAL_COUNT	8
	class GameDBManager
	{
		int areaid;
		int zoneid;
		int deleterole_timeout;
		int max_delta;
		int64_t guid;
		int adduser_on_charge;
		std::vector<region_t> gameserver_region;
		Thread::Mutex	locker_region;
		DBConfig db_config;	// 记录在config表100号记录的服务器数据库创建时间和开服时间
		int create_count;	// 记录服务器启动后创建角色的数量
		DBConfig2 db_config2; //config表101号 记录了DB是否为跨服DB的数据
		bool is_central_db;
		bool is_recall_server;
		bool japan_version;
		DestroyProgram destroy_program;
		int zone_name_max_len;
		typedef std::map<int/*zoneid*/, Octets> ZoneNameMap;
		ZoneNameMap zone_name_map;
	
		typedef std::map<int, point_t> ZONE_POS_MAP;
		ZONE_POS_MAP zone_pos_map;

		GameDBManager() 
		{
			deleterole_timeout = 604800;	// a week
			max_delta = 0;
			guid = 0;
			areaid = 0;
			zoneid = 0;
			adduser_on_charge = 1;
			create_count = 0;
			is_central_db = false;
			is_recall_server = false;
			japan_version = false;
			zone_name_max_len = ZONE_NAME_MAX_LEN_DEFAULT;
		}
	public:
		~GameDBManager() { }
		static GameDBManager* GetInstance() { static GameDBManager instance; return &instance;}
		//initialize role base information when they are created
		bool InitGameDB();
		int GetDeleteRole_Timeout() {return deleterole_timeout;}
		void InitGUID();
		int64_t GetGUID() { return guid++; }
		static int Zoneid() { return GetInstance()->zoneid; }
		static int Areaid() { return GetInstance()->areaid; }
		void SetAddUserOnChange(int value) { adduser_on_charge = value; }
		int  GetAddUserOnChange() { return adduser_on_charge; }
		bool IsFreshServer() { return db_config.open_time == 0 || time(0) - db_config.open_time < 86400; }
		DestroyProgram & GetDestroyProgram(){ return destroy_program; }
		//list 
		void ListUser() { }
		void ListUser(const UserID& userid) { }
		void ListRole() { }
		void ListRole(const RoleId& roleid) { }
		//find 
		int FindGameServer(const point_t& pt);
		//rase details
		bool SaveUserDefault( );
		bool SaveClsDetail(unsigned int id, char gender, GRoleDetail &role);
		bool GetClsDetail(unsigned int id, char gender,
					GRoleBase &base, GRoleStatus &status,
					GRolePocket &pocket, GRoleInventoryVector &equipment,
					GRoleStorehouse &storehouse);
		bool GetClsDetail(unsigned int id, char gender,
					GRoleBase &base, GRoleStatus &status,
					GRolePocket &pocket, GRoleInventoryVector &equipment,
					GRoleStorehouse &storehouse,
					StorageEnv::Storage * pbase,
					StorageEnv::Storage * pstatus,
					StorageEnv::Storage * pinventory,
					StorageEnv::Storage * pequipment,
					StorageEnv::Storage * pstorehouse,
					StorageEnv::Transaction& txn);
		bool GetClsPos(unsigned int id, char gender, float &posx, float &posy, float &posz);

		void UpdateTradeMoney( int money1, int money2 )
		{
			STAT_MIN5("TradeMoney", (int64_t)money1 + money2 );
		}
		void UpdateMoney(int roleid, int delta)
		{
			if(delta>max_delta)
			{
				LOG_TRACE("UpdateMoney, update max delta roleid=%d, delta=%d", roleid,delta);
				max_delta = delta;
			}
			STAT_MIN5("Money",delta);
		}
		void UpdateCash( int delta )
		{
			STAT_MIN5("Cash",delta);
		}

		void ReadDBConfig();
		void SaveDBConfig();
		void OnCreateRole()
		{
			if (++create_count >= 256) {
				create_count = 256;
				if (db_config.open_time == 0)
				{
					db_config.open_time = time(0);
					SaveDBConfig();
				}
			}
			LOG_TRACE("gamedbmanager::OnCreateRole() create_count=%d", create_count);
		}
		bool GetZoneName(int zoneid, Octets& zone_name)
		{
			ZoneNameMap::iterator it = zone_name_map.find(zoneid);
			if (it != zone_name_map.end()) {
				zone_name = it->second;
				return true;
			} else {
				return false;
			}
		}

		void SetZoneNameMaxLen(int value) { zone_name_max_len = value; }
		int LoadZoneNameConfig();
		bool IsCentralDB() { return is_central_db; }
		void SetCentralDB(bool b) { is_central_db = b; }
		bool IsRecallServer() { return is_recall_server; }
		void SetRecallServer(bool b) { is_recall_server = b; }
		bool IsJapanVersion(){ return japan_version; }

		bool IsZoneAccepted(int zoneid) { return zone_pos_map.find(zoneid) != zone_pos_map.end(); }
		bool IsAcceptedZoneListMatch(const std::vector<int>& zones)
		{
			if(zones.size() != zone_pos_map.size()) return false;
			for(unsigned int i = 0; i < zones.size(); ++i)
			{
				if(!IsZoneAccepted(zones[i]))
				{
					Log::log(LOG_ERR, "CrossRelated AnnounceCentralDelivery, zoneid %d is not accpeted, please check the gamesys.conf", zones[i]);
					return false;
				}
			}

			return true;
		}
		
		bool GetZonePos(int zoneid, point_t& pos)
		{
			ZONE_POS_MAP::iterator it = zone_pos_map.find(zoneid);
			if(it != zone_pos_map.end()) {
				pos = it->second;
				return true;
			} else {
				return false;
			}
		}
		bool InitAcceptedZoneCrossPos(const std::string accepted_zones, const std::string cross_zone_pos);
	};

	void GRoleBaseToDetail( const GRoleBase & base, GRoleDetail & detail );
	void GRoleDetailToBase( const GRoleDetail & detail, GRoleBase & base );
};
#endif

