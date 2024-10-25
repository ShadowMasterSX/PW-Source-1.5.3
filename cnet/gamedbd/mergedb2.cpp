#include <sys/types.h>
#include <dirent.h>

#include <set>
#include <map>
#include <vector>
#include <fstream>

#include "macros.h"
#include "accessdb.h"

#include "stocklog"
#include "user"
#include "gfactioninfo"
#include "gmember"
#include "guserfaction"
#include "grolebase"
#include "grolestatus"
#include "groleequipment"
#include "groleinventory"
#include "grolestorehouse"
#include "gshoplog"
#include "gsyslog"
#include "gmailbox"
#include "message"
#include "gfriendinfo"
#include "gfriendlist"
#include "gauctionitem"
#include "gauctiondetail"
#include "grefstore"
#include "grewardstore"
#include "localmacro.h"
#include "gwebtradedetail"
#include "gsysauctioncash"
#include "gfactionfortressdetail"
#include "gfactionrelation"
#include "gforceglobaldatalist"
#include "gfriendextra"
#include "gfriendextinfo"
#include "gsendaumailrecord"
#include "gconsumptionrecord"
#include "pshopdetail"
#include "guniquedataelem"
#ifdef USE_WDB

namespace CMGDB
{

struct lt_Octets
{
	bool operator() (const Octets & o1, const Octets & o2) const
	{
		if( o1.size() < o2.size() )
			return true;
		if( o1.size() > o2.size() )
			return false;
		return memcmp(o1.begin(),o2.begin(),o2.size()) < 0;
	}
};

static int	server2_usercount = 0;
static int	same_usercount = 0;
static int	server2_rolecount = 0;
static int	same_rolecount = 0;

static int	dup_roleid = 0;
static int	dup_factionid = 0;
static int	dup_rolename = 0;
static int	dup_factionname = 0;

// the map relationship
static	std::map<int,int> g_mapRoleid;
static 	std::map<int,int>	g_mapRoleid2Userid;
static	std::map<int,int> g_mapFactionid;
static	std::map<Octets, Octets, lt_Octets>		g_mapRolename;
static	std::map<Octets, Octets, lt_Octets>		g_mapFactionname;

// name prefix and suffix
static	std::string	g_strRoleNamePrefix;
static	std::string	g_strRoleNameSuffix;
static	std::string	g_strFactionNamePrefix;
static	std::string	g_strFactionNameSuffix;
static	Octets		g_octRoleNamePrefix;
static	Octets		g_octRoleNameSuffix;
static	Octets		g_octFactionNamePrefix;
static	Octets		g_octFactionNameSuffix;

static  std::set<int> g_setUserNeedSetReferrer;

static	std::set<int>		g_setLogicuid;
static  std::vector<int> g_need_clear_crossinfo_list;

inline bool IsLogicuidAvailable( int logicuid )
{
	return ( g_setLogicuid.end() == g_setLogicuid.find( logicuid ) );
}
const static int ABNORMAL_LOGICUID_START=(INT_MAX-10000000)&0xfffffff0;
//当源数据库账号的logicuid被使用时，用于分配从ABNORMAL_LOGICUID_START起始的logicuid
int AllocAbnormalLogicuid()
{
	static int logicuid = ABNORMAL_LOGICUID_START;
	while (g_setLogicuid.end()!=g_setLogicuid.find(logicuid) && logicuid<=INT_MAX-16)
		logicuid += 16;
	if (logicuid > INT_MAX-16)
		return -1;
	g_setLogicuid.insert(logicuid);
	return logicuid;
}

class PrepareLogicuidQuery : public StorageEnv::IQuery
{
public:
	bool _cross_locked_role_exsit;

	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			Marshal::OctetsStream	os_key(key), os_value(value);

			int id = -1;
			os_key >> id;
			g_setLogicuid.insert(LOGICUID(id));

			GRoleBase base;
			os_value >> base;
			if(base.status == _ROLE_STATUS_CROSS_LOCKED)
			{
				Log::log(LOG_ERR, "PrepareLogicuidQuery, roleid %d status is _ROLE_STATUS_CROSS_LOCKED", id);

				_cross_locked_role_exsit = true;
				return false;
			}

			if(base.cross_data.size() > 0) g_need_clear_crossinfo_list.push_back(id);
		}
		catch ( ... )
		{
			Log::log( LOG_ERR, "PrepareLogicuidQuery, exception\n" );
		}
		return true;
	}
};

bool PrepareLogicuid( )
{
	printf( "\nPrepare Logicuid:\n" );

	PrepareLogicuidQuery q;
	try
	{
		StorageEnv::Storage * pstorage = StorageEnv::GetStorage( "base" );
		StorageEnv::AtomTransaction	txn;
		try
		{
			q._cross_locked_role_exsit = false;

			StorageEnv::Storage::Cursor cursor = pstorage->cursor(txn);
			cursor.walk( q );

			if(q._cross_locked_role_exsit) 
			{
				printf("MergeDBAll error: CrossLocked Role existed in dest DB\n");
				return false;
			}
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
		Log::log( LOG_ERR, "PrepareLogicuid, error when walk for PrepareLogicuidQuery, what=%s\n", e.what() );
	}
	
	StorageEnv::checkpoint();
	return true;
}

class MergeUserQuery : public StorageEnv::IQuery
{
public:
	StorageEnv::Storage * pstorage;
	MergeUserQuery():pstorage(NULL){}

	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int userid = -1;
				User	user1, user2;
				os_key >> userid;
				if( userid < 32 )
					return true;

				server2_usercount ++;

				os_value2 >> user2;
				if( pstorage->find( key, os_value1, txn ) )
				{
					os_value1 >> user1;
					if( 0 == user1.logicuid && (user1.rolelist&0xFFFF))
					{
						printf("\tERROR:cannot determine logicuid for user1, discard roles, userid=%d,rolelist1=%x\n",
							userid, user1.rolelist);
						user1.rolelist = ROLELIST_DEFAULT;
					}
					if(user1.logicuid && !(user1.rolelist&0xFFFF)) 
						user1.logicuid = 0;

					if(0 == user2.logicuid && (user2.rolelist&0xFFFF))
					{
						printf("\tERROR:cannot determine logicuid for user2, discard roles, userid=%d,rolelist2=%x\n",
							userid, user2.rolelist);
						user2.rolelist = ROLELIST_DEFAULT;
					}
					if(user2.logicuid && !(user2.rolelist&0xFFFF)) 
						user2.logicuid = 0;

					if( !user1.logicuid && user2.logicuid ) 
					{
						if(IsLogicuidAvailable( user2.logicuid ) )
						{
							user1.logicuid = user2.logicuid;
							g_setLogicuid.insert( user2.logicuid );
						}
						else
						{
							int logicuid = AllocAbnormalLogicuid();
							if (logicuid != -1)
							{
								user1.logicuid = logicuid;
								printf( "\tWARN:logicuid already used, alloc abnormal logicuid, userid=%d,abnormal_logicuid=%d,logicuid2=%d,rolelist1=%x,rolelist2=%x\n",
								userid, user1.logicuid, user2.logicuid, user1.rolelist, user2.rolelist );
							}
							else
							{
								printf( "\tERROR:logicuid already used, fail to alloc abnormal logicuid, userid=%d, logicuid1=%d, logicuid2=%d,rolelist1=%x,rolelist2=%x\n",
								userid, user1.logicuid, user2.logicuid, user1.rolelist, user2.rolelist);
								user2.rolelist = ROLELIST_DEFAULT;
							}
						}
					}

					int _same = user1.rolelist & user2.rolelist;
					int _sum  = user1.rolelist | user2.rolelist;
					int _more = (_sum & ~user1.rolelist) | ROLELIST_DEFAULT;
					RoleList r1(user1.rolelist), r2(user2.rolelist), r_same(_same), r_sum(_sum), r_more(_more);

					
					same_usercount ++;
					same_rolecount += r_same.GetRoleCount();

					int role = -1;
					while ( (role = r_more.GetNextRole()) >= 0 )
					{
						g_mapRoleid[user2.logicuid+role] = user1.logicuid+role;
						g_mapRoleid2Userid[user2.logicuid+role] = userid;
						if( user2.logicuid && user2.logicuid!=user1.logicuid )
						{
							printf( "\tWARN:logicuid not equal:userid=%d,roleid1=%d,roleid2=%d\n",
									userid, user1.logicuid+role, user2.logicuid+role );
							dup_roleid ++;
						}
					}
					role = -1;
					while ( (role = r_same.GetNextRole()) >= 0 )
					{
						int newrole = -1;
						if( (newrole = r_sum.AddRole()) >= 0 )
						{
							if( user2.logicuid!=user1.logicuid )
							{
								printf( "\tWARN:logicuid not equal:userid=%d,roleid1=%d,roleid2=%d\n",
										userid, user1.logicuid+role, user2.logicuid+role );
							}
							g_mapRoleid[user2.logicuid+role] = user1.logicuid+newrole;
							g_mapRoleid2Userid[user2.logicuid+role] = userid;
							dup_roleid ++;
						}
						else
						{
							printf( "\tINFO:rolelist full.u=%d,logicuid1=%d,logicuid2=%d,r1=%x,r2=%x,discard=%d,sum=%x.\n",
									userid, user1.logicuid, user2.logicuid, user1.rolelist,
									user2.rolelist, role, r_sum.GetRoleList() );
							g_mapRoleid[user2.logicuid+role] = 0;
							g_mapRoleid2Userid[user2.logicuid+role] = userid;
						}
					}

					user1.rolelist = r_sum.GetRoleList();
					user1.cash += user2.cash;
					user1.money += user2.money;
					user1.cash_add += user2.cash_add;
					user1.cash_buy += user2.cash_buy;
					user1.cash_sell += user2.cash_sell;
					user1.cash_used += user2.cash_used;
					user1.add_serial = std::max(user1.add_serial, user2.add_serial);
					user1.use_serial = std::max(user1.use_serial, user2.use_serial);
					user1.exg_log.insert( user1.exg_log.end(), user2.exg_log.begin(), user2.exg_log.end() );
					while( user1.exg_log.size() > 80 )
						user1.exg_log.erase( user1.exg_log.begin() );
					if( user1.autolock.size() < user2.autolock.size() )
						user1.autolock = user2.autolock;
					if( user1.cash_password.size() == 0 && user2.cash_password.size() > 0 )
						user1.cash_password	=	user2.cash_password;
					//合并系统拍卖元宝
					if(user2.cash_sysauction.size())
					{
						if(user1.cash_sysauction.size())
						{
							GSysAuctionCash sa_cash1, sa_cash2;
							Marshal::OctetsStream(user1.cash_sysauction)>>sa_cash1;
							Marshal::OctetsStream(user2.cash_sysauction)>>sa_cash2;
							sa_cash1.cash_2 	+= sa_cash2.cash_2;
							sa_cash1.cash_used_2 += sa_cash2.cash_used_2;
							user1.cash_sysauction = Marshal::OctetsStream()<<sa_cash1;
						}
						else
							user1.cash_sysauction = user2.cash_sysauction;
					}

					user1.addiction.clear();
					GRoleForbidVector::iterator forbid_it2, forbid_ite2, forbid_it, forbid_ite; 
					for (forbid_it2 = user2.forbid.begin(), forbid_ite2 = user2.forbid.end(); forbid_it2 != forbid_ite2; forbid_it2++)	
					{
						if (forbid_it2->type == FORBID_USER_LOGIN)
						{
							for (forbid_it = user1.forbid.begin(), forbid_ite = user1.forbid.end(); forbid_it != forbid_ite; forbid_it++)
							{
								if (forbid_it->type == FORBID_USER_LOGIN)
								{
									if ( (forbid_it2->createtime+forbid_it2->time) > (forbid_it->createtime+forbid_it->time) )
										*forbid_it = *forbid_it2;
									break;
								}
							}
							if (forbid_it == forbid_ite)
								user1.forbid.push_back(*forbid_it2);
							break;
						}
					}
					//合并上线推广数据
					GRefStore ref1, ref2;
					if (user1.reference.size())
					{
						Marshal::OctetsStream   os_ref(user1.reference);
						os_ref >> ref1;
					}
					if (user2.reference.size())
					{
						Marshal::OctetsStream   os_ref(user2.reference);
						os_ref >> ref2;
					}
					ref1.bonus_used += ref2.bonus_used;	//两个服务器上的bonus_used可能同时大于0，充值返的鸿利被使用时也会增加此值
					if (ref1.bonus_add != 0 && ref2.bonus_add != 0)
						Log::log( LOG_ERR, "Merge Referrer for user %d, bonus_add1 %d bonus_add2 %d", userid, ref1.bonus_add, ref2.bonus_add );
					else
						ref1.bonus_add += ref2.bonus_add;
					if (ref1.referrer != 0 && ref2.referrer != 0)//玩家的上线角色可能被删除置成-1,上线userid一定不为0
						Log::log( LOG_ERR, "Merge Referral for user %d, referrer1 %d referrer2 %d", userid, ref1.referrer, ref2.referrer);
					else if (ref2.referrer != 0)
					{
						ref1.referrer = ref2.referrer;
						ref1.referrer_roleid = ref2.referrer_roleid;
						ref1.bonus_total1 = ref2.bonus_total1;
						ref1.bonus_total2 = ref2.bonus_total2;
						ref1.bonus_withdraw = ref2.bonus_withdraw;
						ref1.bonus_withdraw_today = ref2.bonus_withdraw_today;
						ref1.max_role_level = ref2.max_role_level;
						ref1.rolenames = ref2.rolenames;
						g_setUserNeedSetReferrer.insert(userid);
					}
					if (ref1.bonus_used || ref1.bonus_add || ref1.referrer)
						user1.reference = Marshal::OctetsStream() << ref1;
					//合并充值返利	
					GRewardStore reward1, reward2;
					if (user1.consume_reward.size())
					{
						Marshal::OctetsStream   os(user1.consume_reward);
						os >> reward1;
					}
					if (user2.consume_reward.size())
					{
						Marshal::OctetsStream   os(user2.consume_reward);
						os >> reward2;
						reward1.consume_points += reward2.consume_points;
						reward1.bonus_reward += reward2.bonus_reward;
						GRewardItemVector::const_iterator it, ite;
						GRewardItemVector::iterator it2, ite2;
						for (it=reward2.rewardlist.begin(),ite=reward2.rewardlist.end(); it!=ite; ++it)
						{       
							for (it2=reward1.rewardlist.begin(),ite2=reward1.rewardlist.end(); it2!=ite2; ++it2)
							{       
								if (it->reward_time < it2->reward_time)
								{       
									break;
								}
							}
							reward1.rewardlist.insert(it2, *it);
						}
						user1.consume_reward = Marshal::OctetsStream() << reward1;
					}

					//合并消费值
					GConsumptionRecord consumption1, consumption2;
					if (user1.mall_consumption.size() > 0) {
						Marshal::OctetsStream os(user1.mall_consumption);
						os >> consumption1;
					}
					if (user2.mall_consumption.size() > 0) {
						Marshal::OctetsStream os(user2.mall_consumption);
						os >> consumption2;
						consumption1.consumption += consumption2.consumption;
						user1.mall_consumption = Marshal::OctetsStream() << consumption1;
					}

					//合并账号的任务计数 
					//因为并不知道里面的具体结构，所以选择保留数据量大的 比较弱的策略
					if (user2.taskcounter.size() > user1.taskcounter.size())
						user1.taskcounter = user2.taskcounter;

					pstorage->insert( key, Marshal::OctetsStream()<<user1, txn );
				}
				else
				{
					RoleList r2(user2.rolelist);

					if(user2.logicuid && !(user2.rolelist&0xFFFF))
						user2.logicuid = 0;
					int newlogicuid = user2.logicuid;
					if( user2.logicuid > 0 )
					{
						if( IsLogicuidAvailable( user2.logicuid ) )
							g_setLogicuid.insert( user2.logicuid );
						else
						{
							int logicuid = AllocAbnormalLogicuid();
							if (logicuid != -1)
							{
								newlogicuid = logicuid;
								printf("\tWARN:logicuid already used, alloc abnormal logicuid,userid=%d,abnormal_logicuid=%d,logicuid2=%d,rolelist2=%x\n",
								userid, newlogicuid, user2.logicuid, user2.rolelist );
							}
							else
							{
								printf( "\tERROR:logicuid already used, fail to alloc abnormal logicuid, userid=%d, logicuid2=%d, rolelist2=%x\n",
									userid, user2.logicuid, user2.rolelist );
								r2 = RoleList();
								user2.rolelist = ROLELIST_DEFAULT;
							}
						}
					}
					else if(user2.rolelist&0xFFFF)
					{
						printf("\tERROR:cannot determine logicuid for user2, discard roles, userid=%d,rolelist2=%x\n",
							userid, user2.rolelist);
						r2 = RoleList();
						user2.rolelist = ROLELIST_DEFAULT;
					}

					int role = -1;
					while ( (role = r2.GetNextRole()) >= 0 )
					{
						g_mapRoleid[user2.logicuid+role] = newlogicuid+role;
						g_mapRoleid2Userid[user2.logicuid+role] = userid;
					}

					user2.logicuid = newlogicuid;
					if (user2.reference.size())
					{
						GRefStore ref2;
						Marshal::OctetsStream   os_ref(user2.reference);
						os_ref >> ref2;
						if (ref2.referrer != 0)
							g_setUserNeedSetReferrer.insert(userid);
					}

					pstorage->insert( key, Marshal::OctetsStream()<<user2, txn );
				}
			}
			catch ( DbException e ) { throw; }
			catch ( ... )
			{
				printf("\tWARNNING: MergeUserQuery, unmarshal exception, ignore it!\n");
			}
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "MergeUserQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeUser( const char * srcpath )
{
	printf( "\nMerge user:\n" );

	std::string src_dir = srcpath;

	MergeUserQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/user").c_str() );
		pstandalone->init();
		q.pstorage = StorageEnv::GetStorage("user");

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/user").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeUser, error when walk, what=%s\n", e.what() );
	}
	StorageEnv::checkpoint();
}

class MergeUserStoreQuery : public StorageEnv::IQuery
{
public:
	StorageEnv::Storage * pstorage;
	MergeUserStoreQuery():pstorage(NULL){}

	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int userid = -1;
				GUserStorehouse	store1, store2;
				os_key >> userid;
				if( userid < 32 )
					return true;

				os_value2 >> store2;
				if( pstorage->find( key, os_value1, txn ) )
				{
					os_value1 >> store1;
					if (store2.capacity > store1.capacity)
						store1.capacity = store2.capacity;
					if ((int)(store1.items.size()+store2.items.size()) > store1.capacity*2)
						LOG_TRACE("Merge user %d storehouse %d items will be discard", userid, store1.items.size()+store2.items.size()-store1.capacity*2);
					store1.money += store2.money;
					std::set<int> pos_set;
					GRoleInventoryVector::iterator it, ite;
					for (it=store1.items.begin(),ite=store1.items.end(); it!=ite; ++it)
						pos_set.insert(it->pos);
					GRoleInventoryVector::iterator i2t, i2te;
					int pos = 0;
					for (i2t=store2.items.begin(),i2te=store2.items.end(); (i2t!=i2te && pos<store1.capacity*2); ++i2t)
					{
						while (pos_set.find(pos) != pos_set.end())
							pos++;
						if (pos >= store1.capacity*2)
							break;
						i2t->pos = pos;
						store1.items.push_back(*i2t);
						pos++;
					}
					pstorage->insert( key, Marshal::OctetsStream()<<store1, txn );
				}
				else
				{
					pstorage->insert( key, Marshal::OctetsStream()<<store2, txn );
				}
			}
			catch ( DbException e ) { throw; }
			catch ( ... )
			{
				printf("\tWARNNING: MergeUserStoreQuery, unmarshal exception, ignore it!\n");
			}
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "MergeUserStoreQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeUserStore( const char * srcpath )
{
	printf( "\nMerge user storehouse:\n" );

	std::string src_dir = srcpath;

	MergeUserStoreQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/userstore").c_str() );
		pstandalone->init();
		q.pstorage = StorageEnv::GetStorage("userstore");

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/userstore").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeUserStore, error when walk, what=%s\n", e.what() );
	}
	StorageEnv::checkpoint();
}

void ResetReferrer()
{
	LOG_TRACE("Number of users whoes referrer_roleid need to be reset %d", g_setUserNeedSetReferrer.size());
	try                     
	{               
		StorageEnv::Storage *puser = StorageEnv::GetStorage("user");
		StorageEnv::CommonTransaction txn;
		Marshal::OctetsStream value;
		try     
		{
			std::set<int>::iterator it = g_setUserNeedSetReferrer.begin(), ie = g_setUserNeedSetReferrer.end();
			for (; it != ie; ++it)
			{
				Marshal::OctetsStream key;
				key << *it;
				if (puser->find(key, value, txn))
				{ 
					User user;
					Marshal::OctetsStream(value) >> user;
					if (user.reference.size() == 0)				
					{
						LOG_TRACE("ERR:User %d reference info is NULL", *it);
					}
					else
					{
						bool need_reset = false;
						GRefStore ref_store;
						Marshal::OctetsStream   os_ref(user.reference);
						os_ref >> ref_store;
						if (ref_store.referrer_roleid == 0) //不应该出现的情况
							LOG_TRACE("ERR:old referrer_roleid of user %d is 0", *it);
						else if (ref_store.referrer_roleid == -1)
							LOG_TRACE("old referrer_role of user %d has been deleted", *it);
						else if (g_mapRoleid.find(ref_store.referrer_roleid) == g_mapRoleid.end())
						{
							LOG_TRACE("old referrer_role[%d] for user %d has been deleted, reset to -1", ref_store.referrer_roleid, *it);
							need_reset = true;
							ref_store.referrer_roleid = -1;
						}	
						else
						{
							int oldreferrer = ref_store.referrer_roleid;
							if (g_mapRoleid[oldreferrer] != oldreferrer)
							{
								need_reset = true;
								ref_store.referrer_roleid = g_mapRoleid[oldreferrer];
							}
						}	
						std::vector<Octets>::iterator nit, nite;
						for (nit=ref_store.rolenames.begin(),nite=ref_store.rolenames.end(); nit!=nite;)
						{       
							if (g_mapRolename.find(*nit) == g_mapRolename.end())
							{
								//贡献鸿利的某个下线角色被删除
								LOG_TRACE("erase a referrer rolename for user %d", *it);	
								need_reset = true;
								nit = ref_store.rolenames.erase(nit); 
							}
							else
							{
								Octets oldname = *nit;
								if (g_mapRolename[oldname] != oldname)
								{
									need_reset = true;
									*nit = g_mapRolename[oldname];
								}
								++nit;
							}
						}       
						if (need_reset)
						{
							LOG_TRACE("real reset referrer info for user %d", *it);
							user.reference = Marshal::OctetsStream() << ref_store;
							puser->insert(key, Marshal::OctetsStream()<<user, txn);
						}
					}
				}
				else
					LOG_TRACE("ERR:Can not find user %d", *it);
			}
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
		Log::log( LOG_ERR, "Reset referrer info, error when modify referrer, what=%s\n", e.what() );
	}       
	StorageEnv::checkpoint();
	//q.m_setUserNeedSetReferrer.clear();
}

class MergeRolenameQuery : public StorageEnv::IQuery
{
public:
	StorageEnv::Storage * pstorage;
	MergeRolenameQuery():pstorage(NULL){}

	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int oldroleid = -1, newroleid = -1, status = -1;
				os_value2 >> oldroleid;
				if (os_value2.size() > 4)
					os_value2 >> status;
				newroleid = g_mapRoleid[oldroleid];

				Octets	oldname = key;
				Octets	newname = oldname;

				if( pstorage->find( key, os_value1, txn ) )
				{
					dup_rolename ++;
					int i = 1;
					while( true )
					{
						Octets testname = oldname;
						testname.insert( testname.begin(), g_octRoleNamePrefix.begin(), g_octRoleNamePrefix.size() );
						testname.insert( testname.end(), g_octRoleNameSuffix.begin(), g_octRoleNameSuffix.size() );
						if( i > 1 )
						{
							char buffer[64];
							sprintf( buffer, "%d", i );
							Octets oct_count_local(buffer,strlen(buffer)), oct_count;
							CharsetConverter::conv_charset_l2u( oct_count_local, oct_count );
							testname.insert( testname.end(), oct_count.begin(), oct_count.size() );
						}
						Octets value_tmp;
						if( !pstorage->find( testname, value_tmp, txn ) )
						{
							newname = testname;
							break;
						}
						i ++;
					}
					printf("\tMergeRolename add prefix/suffix automatically oldroleid %d newroleid %d i=%d\n", oldroleid, newroleid, i);
				}

				g_mapRolename.insert( std::make_pair(oldname, newname) );
				if (-1 == status)
					pstorage->insert( newname, Marshal::OctetsStream()<<newroleid, txn );
				else
					pstorage->insert( newname, Marshal::OctetsStream()<<newroleid<<status, txn );
			}
			catch ( DbException e ) { throw; }
			catch ( ... )
			{
				printf("\tWARNNING: MergeRolenameQuery, unmarshal exception, ignore it!\n");
			}
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "MergeRolenameQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeRolename( const char * srcpath )
{
	printf( "\nMerge rolename:\n" );

	std::string src_dir = srcpath;

	MergeRolenameQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/rolename").c_str() );
		pstandalone->init();
		q.pstorage = StorageEnv::GetStorage("rolename");

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/rolename").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeRolename, error when walk, what=%s\n", e.what() );
	}
	StorageEnv::checkpoint();
}

class FindMaxQuery : public StorageEnv::IQuery
{
public:
	int maxid;
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			Marshal::OctetsStream	os_key(key), os_value(value);

			int id = -1;
			os_key >> id;
			if( id > maxid )	maxid	=	id;
		}
		catch ( ... )
		{
			Log::log( LOG_ERR, "FindMaxQuery, exception\n" );
		}
		return true;
	}
};

class MergeFactioninfoQuery : public StorageEnv::IQuery
{
public:
	int maxid;
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("factioninfo");
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int fid = -1, newfid = -1;
				GFactionInfo	info1, info2;
				os_key >> fid;
				os_value2 >> info2;
				if( pstorage->find( key, os_value1, txn ) )
				{	newfid = ++maxid;	dup_factionid ++;	}
				else
					newfid = fid;

				g_mapFactionid[fid] = newfid;
				info2.fid = newfid;
				info2.master.rid = g_mapRoleid[info2.master.rid];
				for( GMemberVector::iterator it = info2.member.begin(), ite = info2.member.end(); it != ite; ++it )
					it->rid = g_mapRoleid[it->rid];

				pstorage->insert( Marshal::OctetsStream()<<newfid, Marshal::OctetsStream()<<info2, txn );
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
			Log::log( LOG_ERR, "MergeFactioninfoQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeFactioninfo( const char * srcpath )
{
	printf( "\nMerge factioninfo:\n" );

	std::string src_dir = srcpath;

	FindMaxQuery fmq;
	fmq.maxid = 0;
	try
	{
		StorageEnv::Storage * pstorage = StorageEnv::GetStorage( "factioninfo" );
		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor = pstorage->cursor(txn);
			cursor.walk( fmq );
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
		Log::log( LOG_ERR, "MergeFactioninfo, error when walk for FindMaxQuery, what=%s\n", e.what() );
	}

	MergeFactioninfoQuery q;
	q.maxid = fmq.maxid;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/factioninfo").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/factioninfo").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeFactioninfo, error when walk for MergeFactioninfoQuery, what=%s\n", e.what() );
	}
}

class MergeFactionnameQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("factionname");
			StorageEnv::Storage * pfactioninfo = StorageEnv::GetStorage("factioninfo");
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int oldfactionid = -1;
				int newfactionid = -1;
				os_value2 >> oldfactionid;
				newfactionid = g_mapFactionid[oldfactionid];

				Octets	oldname;
				os_key >> oldname;
				Octets	newname = oldname;

				if( pstorage->find( key, os_value1, txn ) )
				{
					dup_factionname ++;
					int i = 1;
					while( true )
					{
						Octets testname = oldname;
						testname.insert( testname.begin(), g_octFactionNamePrefix.begin(), g_octFactionNamePrefix.size() );
						testname.insert( testname.end(), g_octFactionNameSuffix.begin(), g_octFactionNameSuffix.size() );
						if( i > 1 )
						{
							char buffer[64];
							sprintf( buffer, "%d", i );
							Octets oct_count_local(buffer,strlen(buffer)), oct_count;
							CharsetConverter::conv_charset_l2u( oct_count_local, oct_count );
							testname.insert( testname.end(), oct_count.begin(), oct_count.size() );
						}

						Octets value_tmp;
						if( !pstorage->find( Marshal::OctetsStream()<<testname, value_tmp, txn ) )
						{
							newname = testname;
							break;
						}
						i ++;
					}

					Marshal::OctetsStream	os_factioninfo;
					if( pfactioninfo->find( Marshal::OctetsStream()<<newfactionid, os_factioninfo, txn ) )
					{
						GFactionInfo	info;
						os_factioninfo >> info;
						info.name	=	newname;
						pfactioninfo->insert( Marshal::OctetsStream()<<newfactionid, Marshal::OctetsStream()<<info, txn );
					}
					
					printf("\tMergeFactionname add prefix/suffix automatically oldfactionid %d newfactionid %d i=%d\n", oldfactionid, newfactionid, i);
				}

				g_mapFactionname.insert( std::make_pair(oldname, newname) );
				pstorage->insert( Marshal::OctetsStream()<<newname, Marshal::OctetsStream()<<newfactionid, txn );
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
			Log::log( LOG_ERR, "MergeFactionnameQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeFactionname( const char * srcpath )
{
	printf( "\nMerge factionname:\n" );

	std::string src_dir = srcpath;

	MergeFactionnameQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/factionname").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/factionname").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeFactionname, error when walk, what=%s\n", e.what() );
	}
}

class MergeBaseQuery : public StorageEnv::IQuery
{
public:
	StorageEnv::Storage * pstorage;
	int count;
	bool hasmore; 
	Octets handle;
	MergeBaseQuery():pstorage(NULL),count(0),hasmore(false){}
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		if(++count % 10000 == 0)
		{
			handle = key;
			hasmore = true;
			return false;
		}
		try
		{
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);
				
				int oldroleid = -1, newroleid = -1;
				os_key >> oldroleid;
				if( oldroleid < 32 )
					return true;
				newroleid = g_mapRoleid[oldroleid];
				GRoleBase	base;
				os_value2 >> base;
				if( 0 == newroleid )
				{
					printf("\tERROR:role missed in User.rolelist or logicuid not available,skip roleid=%d(userid=%d).\n",
						oldroleid, base.userid);
					return true;
				}

				server2_rolecount ++;
				base.id		=	newroleid;
				if (base.userid == 0)
				{
					base.userid = g_mapRoleid2Userid[oldroleid];
				}
				base.spouse	=	g_mapRoleid[base.spouse];
				if( g_mapRolename[base.name].size() > 0 )
					base.name	=	g_mapRolename[base.name];

				base.cross_data.clear(); //merge前需要清空base里的跨服信息
				
				if( pstorage->find( Marshal::OctetsStream()<<newroleid, os_value1, txn ) )
					printf( "\tWARN:MergeBaseQuery Error, overwrite base. oldroleid = %d, newroleid = %d.\n",oldroleid,newroleid );
				pstorage->insert( Marshal::OctetsStream()<<newroleid, Marshal::OctetsStream()<<base, txn );
			}
			catch ( DbException e ) { throw; }
			catch ( ... )
			{
				printf("\tWARNNING: MergeBaseQuery, unmarshal exception, ignore it!\n");
			}
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "MergeBaseQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeBase( const char * srcpath )
{
	printf( "\nMerge base:\n" );

	std::string src_dir = srcpath;

	MergeBaseQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/base").c_str() );
		pstandalone->init();

		do{	
			q.hasmore = false;
			
			{
				q.pstorage = StorageEnv::GetStorage("base");
				StorageEnv::AtomTransaction	txn;
				try
				{
					StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/base").c_str(),pstandalone,new StorageEnv::Uncompressor());
					cursor.walk( q.handle, q );
				}
				catch ( DbException e ) { throw; }
				catch ( ... )
				{
					DbException ee( DB_OLD_VERSION );
					txn.abort( ee );
					throw ee;
				}
			}
			
			pstandalone->checkpoint();
			StorageEnv::checkpoint();	
		}while(q.hasmore);

		delete pstandalone;
		pstandalone = NULL;
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeBase, error when walk, what=%s\n", e.what() );
	}
}

class MergeRoleDataQuery : public StorageEnv::IQuery
{
public:
	std::string	tablename;
	StorageEnv::Storage * pstorage;
	int count; 
	bool hasmore;
	Octets handle;
	MergeRoleDataQuery():pstorage(NULL),count(0),hasmore(false){}
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		if(++count % 10000 == 0)
		{
			handle = key;
			hasmore = true;
			return false;
		}
		try
		{
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int oldroleid = -1, newroleid = -1;
				os_key >> oldroleid;
				newroleid = g_mapRoleid[oldroleid];
				if( oldroleid < 32 || 0 == newroleid )
					return true;

				if( pstorage->find( Marshal::OctetsStream()<<newroleid, os_value1, txn ) )
					printf( "\tWARN:MergeRoleDataQuery Error, overwrite %s. oldroleid = %d, newroleid = %d.\n",
								tablename.c_str(), oldroleid, newroleid );

				pstorage->insert( Marshal::OctetsStream()<<newroleid, value, txn );
			}
			catch ( DbException e ) { throw; }
			catch ( ... )
			{
				printf("\tWARNNING: MergeRoleDataQuery, unmarshal exception, ignore it!\n");
			}
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "MergeRoleDataQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeRoleData( const char * srcpath, const char * tablename )
{
	printf( "\nMerge %s:\n", tablename );

	std::string src_dir = srcpath;

	MergeRoleDataQuery q;
	q.tablename = tablename;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/"+tablename).c_str() );
		pstandalone->init();

		do{
			q.hasmore = false;
			
			{
				q.pstorage = StorageEnv::GetStorage( tablename );
				StorageEnv::AtomTransaction	txn;
				try
				{
					StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/"+tablename).c_str(),pstandalone,new StorageEnv::Uncompressor());
					cursor.walk( q.handle, q );
				}
				catch ( DbException e ) { throw; }
				catch ( ... )
				{
					DbException ee( DB_OLD_VERSION );
					txn.abort( ee );
					throw ee;
				}
			}

			pstandalone->checkpoint();
			StorageEnv::checkpoint();
		}while(q.hasmore);
		
		delete pstandalone;
		pstandalone = NULL;
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeRoleData, error when walk, what=%s\n", e.what() );
	}
}

class MergeMailboxQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("mailbox");
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int oldroleid = -1, newroleid = -1;
				os_key >> oldroleid;
				newroleid = g_mapRoleid[oldroleid];
				if( oldroleid < 32 || 0 == newroleid )
					return true;

				GMailBox	box;
				os_value2 >> box;
				for( GMailVector::iterator it = box.mails.begin(), ite = box.mails.end(); it != ite; ++it )
				{
					if(it->header.sender >= 32)
						it->header.sender = g_mapRoleid[it->header.sender];
					//receiver已经不是接受者id
					//it->header.receiver = g_mapRoleid[it->header.receiver];
					it->header.sender_name = g_mapRolename[it->header.sender_name];
				}
				if( pstorage->find( Marshal::OctetsStream()<<newroleid, os_value1, txn ) )
					printf( "\tWARN:MergeMailboxQuery Error, overwrite mailbox. oldroleid = %d, newroleid = %d.\n",
								oldroleid, newroleid );

				pstorage->insert( Marshal::OctetsStream()<<newroleid, Marshal::OctetsStream()<<box, txn );
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
			Log::log( LOG_ERR, "MergeMailboxQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeMailbox( const char * srcpath )
{
	printf( "\nMerge mailbox:\n" );

	std::string src_dir = srcpath;

	MergeMailboxQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/mailbox").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/mailbox").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeMailbox, error when walk, what=%s\n", e.what() );
	}
}

class MergeMessagesQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("messages");
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int oldroleid = -1, newroleid = -1;
				os_key >> oldroleid;
				newroleid = g_mapRoleid[oldroleid];
				if( oldroleid < 32 || 0 == newroleid )
					return true;

				MessageVector	msgs;
				os_value2 >> msgs;
				for( MessageVector::iterator it = msgs.begin(), ite = msgs.end(); it != ite; ++it )
				{
					it->srcroleid = g_mapRoleid[it->srcroleid];
					it->dstroleid = g_mapRoleid[it->dstroleid];
					it->src_name  = g_mapRolename[it->src_name];
					it->dst_name  = g_mapRolename[it->dst_name];
				}
				if( pstorage->find( Marshal::OctetsStream()<<newroleid, os_value1, txn ) )
					printf( "\tWARN:MergeMessagesQuery Error, overwrite messages. oldroleid = %d, newroleid = %d.\n",
								oldroleid, newroleid );

				pstorage->insert( Marshal::OctetsStream()<<newroleid, Marshal::OctetsStream()<<msgs, txn );
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
			Log::log( LOG_ERR, "MergeMessagesQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeMessages( const char * srcpath )
{
	printf( "\nMerge messages:\n" );

	std::string src_dir = srcpath;

	MergeMessagesQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/messages").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/messages").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeMessages, error when walk, what=%s\n", e.what() );
	}
}

class MergeFriendsQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("friends");
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int oldroleid = -1, newroleid = -1;
				os_key >> oldroleid;
				newroleid = g_mapRoleid[oldroleid];
				if( oldroleid < 32 || 0 == newroleid )
					return true;

				GFriendList	fl;
				os_value2 >> fl;
				for( GFriendInfoVector::iterator it = fl.friends.begin(), ite = fl.friends.end(); it != ite; ++it )
				{
					it->rid		=	g_mapRoleid[it->rid];
					it->name	=	g_mapRolename[it->name];
				}
				if( pstorage->find( Marshal::OctetsStream()<<newroleid, os_value1, txn ) )
					printf( "\tWARN:MergeFriendsQuery Error, overwrite friends. oldroleid = %d, newroleid = %d.\n",
								oldroleid, newroleid );

				pstorage->insert( Marshal::OctetsStream()<<newroleid, Marshal::OctetsStream()<<fl, txn );
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
			Log::log( LOG_ERR, "MergeFriendsQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeFriends( const char * srcpath )
{
	printf( "\nMerge friends:\n" );

	std::string src_dir = srcpath;

	MergeFriendsQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/friends").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/friends").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeFriends, error when walk, what=%s\n", e.what() );
	}
}

class MergePlayerShopQuery : public StorageEnv::IQuery
{
public:
	StorageEnv::Storage * pstorage;
	MergePlayerShopQuery():pstorage(NULL){}
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int oldroleid = -1, newroleid = -1;
				os_key >> oldroleid;
				newroleid = g_mapRoleid[oldroleid];
				if( oldroleid < 32 || 0 == newroleid )
					return true;

				PShopDetail	ps;
				os_value2 >> ps;
				ps.roleid = newroleid;
				
				if( pstorage->find( Marshal::OctetsStream()<<newroleid, os_value1, txn ) )
					printf( "\tWARN:MergePlayerShopQuery Error, overwrite pshop. oldroleid = %d, newroleid = %d.\n",
								oldroleid, newroleid );

				pstorage->insert( Marshal::OctetsStream()<<newroleid, Marshal::OctetsStream()<<ps, txn );
			}
			catch ( DbException e ) { throw; }
			catch ( ... )
			{
				printf("\tWARNNING: MergePlayerShopQuery, unmarshal exception, ignore it!\n");
			}
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "MergePlayerShopQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergePlayerShop( const char * srcpath )
{
	printf( "\nMerge playershop:\n" );

	std::string src_dir = srcpath;

	MergePlayerShopQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/playershop").c_str() );
		pstandalone->init();
		q.pstorage = StorageEnv::GetStorage("playershop");

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/playershop").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergePlayerShop, error when walk, what=%s\n", e.what() );
	}
	StorageEnv::checkpoint();
}

class MergeUserfactionQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("userfaction");
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int oldroleid = -1, newroleid = -1;
				os_key >> oldroleid;
				newroleid = g_mapRoleid[oldroleid];
				if( oldroleid < 32 || 0 == newroleid )
					return true;

				GUserFaction	uf;
				os_value2 >> uf;
				uf.rid	=	g_mapRoleid[uf.rid];
				uf.name	=	g_mapRolename[uf.name];
				uf.fid	=	g_mapFactionid[uf.fid];
				
				if( pstorage->find( Marshal::OctetsStream()<<newroleid, os_value1, txn ) )
					printf( "\tWARN:MergeUserfactionQuery Error, overwrite userfaction. oldroleid = %d, newroleid = %d.\n",
								oldroleid, newroleid );

				pstorage->insert( Marshal::OctetsStream()<<newroleid, Marshal::OctetsStream()<<uf, txn );
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
			Log::log( LOG_ERR, "MergeUserfactionQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeUserfaction( const char * srcpath )
{
	printf( "\nMerge userfaction:\n" );

	std::string src_dir = srcpath;

	MergeUserfactionQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/userfaction").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/userfaction").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeUserfaction, error when walk, what=%s\n", e.what() );
	}
}

class MergeFactionfortressQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("factionfortress");
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int oldfactionid = -1, newfactionid = -1;
				os_key >> oldfactionid;
				newfactionid = g_mapFactionid[oldfactionid];
				if(0 == newfactionid)
					return true;

				GFactionFortressDetail detail;
				os_value2 >> detail;
				detail.factionid = newfactionid;
				if(detail.info2.offense_faction)
					detail.info2.offense_faction = g_mapFactionid[detail.info2.offense_faction];
				for(size_t i=0; i<detail.info2.challenge_list.size(); i++)
				{
					detail.info2.challenge_list[i] = g_mapFactionid[ detail.info2.challenge_list[i] ];	
				}
				
				if( pstorage->find( Marshal::OctetsStream()<<newfactionid, os_value1, txn ) )
					printf( "\tWARN:MergeFactionfortressQuery Error, overwrite factionfortress. oldfactionid = %d, newfactionid = %d.\n",
								oldfactionid, newfactionid);

				pstorage->insert( Marshal::OctetsStream()<<newfactionid, Marshal::OctetsStream()<<detail, txn );
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
			Log::log( LOG_ERR, "MergeFactionfortressQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeFactionfortress( const char * srcpath )
{
	printf( "\nMerge factionfortress:\n" );

	std::string src_dir = srcpath;

	MergeFactionfortressQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/factionfortress").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/factionfortress").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeFactionfortress, error when walk, what=%s\n", e.what() );
	}
}

class MergeFactionrelationQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("factionrelation");
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int oldfactionid = -1, newfactionid = -1;
				os_key >> oldfactionid;
				newfactionid = g_mapFactionid[oldfactionid];
				if(0 == newfactionid)
					return true;

				GFactionRelation relation;
				os_value2 >> relation;
				relation.fid = newfactionid;
				for(size_t i=0; i<relation.alliance.size(); i++)
					relation.alliance[i].fid = g_mapFactionid[ relation.alliance[i].fid ];
				for(size_t i=0; i<relation.hostile.size(); i++)
					relation.hostile[i].fid = g_mapFactionid[ relation.hostile[i].fid ];
				for(size_t i=0; i<relation.apply.size(); i++)
					relation.apply[i].fid = g_mapFactionid[ relation.apply[i].fid ];

				if( pstorage->find( Marshal::OctetsStream()<<newfactionid, os_value1, txn ) )
					printf( "\tWARN:MergeFactionrelationQuery Error, overwrite factionrelation. oldfactionid = %d, newfactionid = %d.\n",
								oldfactionid, newfactionid);

				pstorage->insert( Marshal::OctetsStream()<<newfactionid, Marshal::OctetsStream()<<relation, txn );
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
			Log::log( LOG_ERR, "MergeFactionrelationQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeFactionrelation( const char * srcpath )
{
	printf( "\nMerge factionrelation:\n" );

	std::string src_dir = srcpath;

	MergeFactionrelationQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/factionrelation").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/factionrelation").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeFactionrelation, error when walk, what=%s\n", e.what() );
	}
}

class MergeAuctionQuery : public StorageEnv::IQuery
{
public:
	int	maxid;
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("auction");
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				int id = ++maxid;
				GAuctionDetail	auction;
				os_value2 >> auction;
				auction.info.auctionid = id;
				auction.info.seller = g_mapRoleid[auction.info.seller];
				auction.info.bidder = g_mapRoleid[auction.info.bidder];
				pstorage->insert( Marshal::OctetsStream()<<id, Marshal::OctetsStream()<<auction, txn );
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
			Log::log( LOG_ERR, "MergeAuctionQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeAuction( const char * srcpath )
{
	printf( "\nMerge auction:\n" );

	std::string src_dir = srcpath;

	FindMaxQuery fmq;
	fmq.maxid = 0;
	try
	{
		StorageEnv::Storage * pstorage = StorageEnv::GetStorage( "auction" );
		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor = pstorage->cursor(txn);
			cursor.walk( fmq );
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
		Log::log( LOG_ERR, "MergeAuction, error when walk for FindMaxQuery, what=%s\n", e.what() );
	}

	MergeAuctionQuery q;
	q.maxid = fmq.maxid;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/auction").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/auction").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeAuction, error when walk for MergeAuctionQuery, what=%s\n", e.what() );
	}
}

class MergeShoplogQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("shoplog");
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				std::vector<GShopLog>	vsl;
				os_value2 >> vsl;
				for( std::vector<GShopLog>::iterator it = vsl.begin(), ite = vsl.end(); it != ite; ++it )
					it->roleid	=	g_mapRoleid[it->roleid];

				int64_t	shoplogid = 0, shoplogidnew = 0;
				static int64_t shoplogid_inc = 0;
				os_key >> shoplogid;
				shoplogidnew = shoplogid + shoplogid_inc;

				while( true )
				{
					if( !pstorage->find( Marshal::OctetsStream()<<shoplogidnew, os_value1, txn ) )
					{
						pstorage->insert( Marshal::OctetsStream()<<shoplogidnew, Marshal::OctetsStream()<<vsl, txn );
						break;
					}
					shoplogid_inc	+= 0x0000000100000000LL;
					shoplogidnew	+= shoplogid_inc;
				}
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
			Log::log( LOG_ERR, "MergeShoplogQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeShoplog( const char * srcpath )
{
	printf( "\nMerge shoplog:\n" );

	std::string src_dir = srcpath;

	MergeShoplogQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/shoplog").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/shoplog").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeShoplog, error when walk, what=%s\n", e.what() );
	}
}

class MergeSyslogQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("syslog");
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);

				GSysLog	log;
				os_value2 >> log;
				log.roleid	=	g_mapRoleid[log.roleid];

				int64_t	syslogid = 0, syslogidnew = 0;
				static int64_t syslogid_inc = 0;
				os_key >> syslogid;
				syslogidnew = syslogid + syslogid_inc;

				while( true )
				{
					if( !pstorage->find( Marshal::OctetsStream()<<syslogidnew, os_value1, txn ) )
					{
						pstorage->insert( Marshal::OctetsStream()<<syslogidnew, Marshal::OctetsStream()<<log, txn );
						break;
					}
					syslogid_inc	+= 0x0000000100000000LL;
					syslogidnew		+= syslogid_inc;
				}
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
			Log::log( LOG_ERR, "MergeSyslogQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeSyslog( const char * srcpath )
{
	printf( "\nMerge syslog:\n" );

	std::string src_dir = srcpath;

	MergeSyslogQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/syslog").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/syslog").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeSyslog, error when walk, what=%s\n", e.what() );
	}
}

class MergeWebTradeQuery : public StorageEnv::IQuery
{
public:
	std::string tablename;
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage(tablename.c_str());
			try
			{
				Marshal::OctetsStream os_key(key), os_value(value);

				int64_t sn;
				os_key >> sn;
				if(sn == 0)
				{
					int64_t max_sn = 0;
					os_value >> max_sn;
					int64_t max_sn2 = 0;
					Marshal::OctetsStream os_value2;
					if(pstorage->find(key, os_value2, txn))
						os_value2 >> max_sn2;
					if((max_sn & 0xFFFFFFFF) > (max_sn2 & 0xFFFFFFFF))
						pstorage->insert( key, Marshal::OctetsStream()<<max_sn, txn);
					return true;
				}
				GWebTradeDetail	detail;
				os_value >> detail;
				detail.info.seller_roleid = g_mapRoleid[detail.info.seller_roleid];
				detail.info.buyer_roleid = g_mapRoleid[detail.info.buyer_roleid];
				detail.info.seller_name = g_mapRolename[detail.info.seller_name];
				detail.info.buyer_name = g_mapRolename[detail.info.buyer_name];
				pstorage->insert( key, Marshal::OctetsStream()<<detail, txn, DB_NOOVERWRITE);
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
			Log::log( LOG_ERR, "MergeWebTradeQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeWebTrade( const char * srcpath, const char * tablename )
{
	printf( "\nMerge %s:\n", tablename );

	std::string src_dir = srcpath;

	MergeWebTradeQuery q;
	q.tablename = tablename;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/"+tablename).c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/"+tablename).c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeWebTrade, error when walk for MergeWebTradeQuery, what=%s\n", e.what() );
	}
}

class FindOldWebTradeSoldQuery : public StorageEnv::IQuery
{
public:
	FindOldWebTradeSoldQuery(){ cur_time = 1000LL*time(NULL); }
	int64_t cur_time;	//当前时间的ms数
	std::set<int64_t> del_keys;

	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			Marshal::OctetsStream	os_key(key), os_value(value);

			int64_t sn;
			GWebTradeDetail	detail;
			os_key >> sn;
			os_value >> detail;
			if(detail.post_time < cur_time - 2592000000LL)	//删除一个月前的记录
				del_keys.insert(sn);
		}
		catch ( ... )
		{
			Log::log( LOG_ERR, "FindOldWebTradeSoldQuery, exception\n" );
		}
		return true;
	}
};

void DelOldWebTradeSold()
{
	printf("\nDel old webtrade sold record:");	
	
	FindOldWebTradeSoldQuery q;
	try
	{
		StorageEnv::Storage * pstorage = StorageEnv::GetStorage( "webtradesold" );
		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor = pstorage->cursor(txn);
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		
		printf(" count=%d\n", q.del_keys.size());
		//del
		std::set<int64_t>::iterator it=q.del_keys.begin(), ie=q.del_keys.end();
		for( ; it!=ie ; ++it)
			pstorage->del(Marshal::OctetsStream() << *it, txn);	
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "DelOldWebTradeSold, error when walk for FindOldWebTradeSoldQuery, what=%s\n", e.what() );
	}
}

class MergeForceQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("force");
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);
				
				int tmp;
				os_key >> tmp;
				if(tmp != 0) return true;

				if( pstorage->find( os_key, os_value1, txn ) )
				{
					GForceGlobalDataList list1, list2;
					os_value1 >> list1;
					os_value2 >> list2;
					for(size_t i=0; i<list2.list.size(); i++)
					{
						GForceGlobalData & data2 = list2.list[i];
						bool find = false;
						for(size_t j=0; j<list1.list.size(); j++)
						{
							GForceGlobalData & data1 = list1.list[j];
							if(data1.force_id == data2.force_id)
							{
								data1.player_count += data2.player_count;
								data1.development += data2.development;
								data1.construction += data2.construction;
								data1.activity += data2.activity;
								find = true;
								break;
							}							
						}
						if(!find) list1.list.push_back(data2);
					}
					//清空活跃度等级
					for(size_t j=0; j<list1.list.size(); j++)
					{
						GForceGlobalData & data1 = list1.list[j];
						data1.activity_level = 0;
					}
					pstorage->insert( os_key, Marshal::OctetsStream()<<list1, txn );
				}
				else
				{
					pstorage->insert( os_key, os_value2, txn );
				}
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
			Log::log( LOG_ERR, "MergeForceQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

class MergeFriendExtQuery : public StorageEnv::IQuery
{
public:
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("friendext");
			try
			{
				Marshal::OctetsStream os_key(key),os_value1,os_value2(value);

				int oldroleid = -1,newroleid = -1;
				os_key >> oldroleid;
				newroleid = g_mapRoleid[oldroleid];
				if( oldroleid < 32 || 0 == newroleid )
					return true;

				GFriendExtra fe;
				os_value2 >> fe;
				for( GFriendExtInfoVector::iterator it = fe.friendExtInfo.begin();it!=fe.friendExtInfo.end();++it)
				{
					it->rid = g_mapRoleid[it->rid];
				}
				for( GSendAUMailRecordVector::iterator it = fe.sendaumailinfo.begin();it!=fe.sendaumailinfo.end();++it)
				{
					it->rid = g_mapRoleid[it->rid];
				}
				if( pstorage->find( Marshal::OctetsStream()<<newroleid, os_value1, txn ) )
					printf( "\tWARN:MergeFriendExtQuery Error, overwrite friends. oldroleid = %d, newroleid = %d.\n",
								oldroleid, newroleid );

				pstorage->insert( Marshal::OctetsStream()<<newroleid, Marshal::OctetsStream()<<fe, txn );
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
			Log::log( LOG_ERR, "MergeFriendExtQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeForce( const char * srcpath )
{
	printf( "\nMerge force:\n" );

	std::string src_dir = srcpath;

	MergeForceQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/force").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/force").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeForce, error when walk, what=%s\n", e.what() );
	}
}

void MergeFriendExt( const char * srcpath)
{
	printf( "\nMerge firendext:\n" );

	std::string src_dir = srcpath;

	MergeFriendExtQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/friendext").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/friendext").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeFriendExt, error when walk, what=%s\n", e.what() );
	}
}

class MergeUniqueDataQuery : public StorageEnv::IQuery
{
public:
	GUniqueDataElem& IntGreater(int uid,GUniqueDataElem& e1,GUniqueDataElem& e2)
	{
		if(e1.vtype != UDT_INT || e2.vtype != UDT_INT)	
		{
			Log::log( LOG_ERR, "IntGreater,uniquedata%d type err srctype%d desttype%d\n",uid, e1.vtype ,e2.vtype);
			return e1;
		}

		return *((int*)e1.value.begin()) >= *((int*)e2.value.begin()) ? e1 : e2;
	}

	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("uniquedata");
			try
			{
				Marshal::OctetsStream	os_key(key), os_value1, os_value2(value);
				
				int uid;
				os_key >> uid;

				if( pstorage->find( os_key, os_value1, txn ) )
				{
					GUniqueDataElem elem1, elem2;
					os_value1 >> elem1;
					os_value2 >> elem2;

					if(elem1.vtype != elem2.vtype)
					{
						Log::log( LOG_ERR, "MergeUniqueDataQuery,uniquedata%d type diff srctype%d desttype%d\n",uid, elem1.vtype ,elem2.vtype);
						return true;
					}

					switch(uid)
					{
						case 0:
							{
								elem1 = IntGreater(uid,elem1,elem2);
							}
							break;
						default:
							{
								if(uid >= 10001 && uid <= 11000)
								{
									elem1 = IntGreater(uid,elem1,elem2);
								}
							}	
							break;
					}

					pstorage->insert( os_key, Marshal::OctetsStream()<<elem1, txn );
				}
				else
				{
					pstorage->insert( os_key, os_value2, txn );
				}
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
			Log::log( LOG_ERR, "MergeUniqueDataQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeUniqueData( const char * srcpath)
{
	printf( "\nMerge uniquedata:\n" );

	std::string src_dir = srcpath;

	MergeUniqueDataQuery q;
	try
	{
		DBStandalone * pstandalone = new DBStandalone( (src_dir+"/uniquedata").c_str() );
		pstandalone->init();

		StorageEnv::AtomTransaction txn;
		try
		{
			StorageEnv::Storage::Cursor cursor(&txn,(src_dir+"/uniquedata").c_str(),pstandalone,new StorageEnv::Uncompressor());
			cursor.walk( q );
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
		pstandalone->checkpoint();
		delete pstandalone;
		pstandalone = NULL;
		StorageEnv::checkpoint();
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "MergeUniqueData, error when walk, what=%s\n", e.what() );
	}
}

void LoadConfig( )
{
	// load name prefix and suffix
	g_strRoleNamePrefix		=	Conf::GetInstance()->find( "gamedbd", "rolenameprefix" );
	g_strRoleNameSuffix		=	Conf::GetInstance()->find( "gamedbd", "rolenamesuffix" );
	g_strFactionNamePrefix	=	Conf::GetInstance()->find( "gamedbd", "factionnameprefix" );
	g_strFactionNameSuffix	=	Conf::GetInstance()->find( "gamedbd", "factionnamesuffix" );

	Octets      l_octRoleNamePrefix( g_strRoleNamePrefix.c_str(), g_strRoleNamePrefix.length() );
	Octets      l_octRoleNameSuffix( g_strRoleNameSuffix.c_str(), g_strRoleNameSuffix.length() );
	Octets      l_octFactionNamePrefix( g_strFactionNamePrefix.c_str(), g_strFactionNamePrefix.length() );
	Octets      l_octFactionNameSuffix( g_strFactionNameSuffix.c_str(), g_strFactionNameSuffix.length() );

	CharsetConverter::conv_charset_l2u( l_octRoleNamePrefix, g_octRoleNamePrefix );
	CharsetConverter::conv_charset_l2u( l_octRoleNameSuffix, g_octRoleNameSuffix );
	CharsetConverter::conv_charset_l2u( l_octFactionNamePrefix, g_octFactionNamePrefix );
	CharsetConverter::conv_charset_l2u( l_octFactionNameSuffix, g_octFactionNameSuffix );

	fprintf( stderr, "\nLoad Configuration:\n" );
	fprintf( stderr, "\tINFO:role name prefix(local):" );		l_octRoleNamePrefix.dump();
	fprintf( stderr, "\tINFO:role name suffix(local):" );		l_octRoleNameSuffix.dump();
	fprintf( stderr, "\tINFO:faction name prefix(local):" );		l_octFactionNamePrefix.dump();
	fprintf( stderr, "\tINFO:faction name suffix(local):" );		l_octFactionNameSuffix.dump();

	fprintf( stderr, "\tINFO:role name prefix(UTF-16LE):" );		g_octRoleNamePrefix.dump();
	fprintf( stderr, "\tINFO:role name suffix(UTF-16LE):" );		g_octRoleNameSuffix.dump();
	fprintf( stderr, "\tINFO:faction name prefix(UTF-16LE):");	g_octFactionNamePrefix.dump();
	fprintf( stderr, "\tINFO:faction name suffix(UTF-16LE):");	g_octFactionNameSuffix.dump();
}

void PrintMap( )
{
	{
		printf( "g_mapRoleid size=%d:\n", g_mapRoleid.size() );
		for( std::map<int,int>::iterator it=g_mapRoleid.begin(), ite=g_mapRoleid.end(); it != ite; ++it )
		{
			//if( it->first != it->second )
				printf( "%d,%d\n", it->first, it->second );
		}
	}

	{
		printf( "g_mapFactionid size=%d:\n", g_mapFactionid.size() );
		for( std::map<int,int>::iterator it=g_mapFactionid.begin(), ite=g_mapFactionid.end(); it != ite; ++it )
		{
			//if( it->first != it->second )
				printf( "%d,%d\n", it->first, it->second );
		}
	}

	{
		printf( "g_mapRolename size=%d:\n", g_mapRolename.size() );
		for( std::map<Octets, Octets, lt_Octets>::iterator it=g_mapRolename.begin(), ite=g_mapRolename.end(); it != ite; ++it )
		{
			//if( it->first != it->second )
			{
				Octets	n1 = it->first, n2 = it->second;
				Octets	name1, name2;
				CharsetConverter::conv_charset_u2l( n1, name1 );
				CharsetConverter::conv_charset_u2l( n2, name2 );
				printf( "%.*s,%.*s\n", name1.size(), (char*)name1.begin(), name2.size(), (char*)name2.begin() );
			}
		}
	}

	{
		printf( "g_mapFactionname size=%d:\n", g_mapFactionname.size() );
		for(std::map<Octets, Octets, lt_Octets>::iterator it=g_mapFactionname.begin(), ite=g_mapFactionname.end(); it != ite; ++it)
		{
			//if( it->first != it->second )
			{
				Octets	n1 = it->first, n2 = it->second;
				Octets	name1, name2;
				CharsetConverter::conv_charset_u2l( n1, name1 );
				CharsetConverter::conv_charset_u2l( n2, name2 );
				printf( "%.*s,%.*s\n", name1.size(), (char*)name1.begin(), name2.size(), (char*)name2.begin() );
			}
		}
	}
}

class CheckCrossLockedQuery: public StorageEnv::IQuery
{
public:
	bool _cross_locked_role_exsit;
	
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			Marshal::OctetsStream os_key(key), os_value(value);
			Marshal::OctetsStream os_base;
			
			int id = -1;
			os_key >> id;
			GRoleBase base;
			os_value >> base;
			
			if(base.status == _ROLE_STATUS_CROSS_LOCKED)
			{
				Log::log(LOG_ERR, "CheckCrossLockedRoleQuery, roleid %d status is _ROLE_STATUS_CROSS_LOCKED", id);
				_cross_locked_role_exsit = true;
				return false;
			}
		}
		catch ( Marshal::Exception e )
		{
			Log::log(LOG_ERR, "CheckCrossLockedRoleQuery, exception\n");
			throw e;
		}
		return true;
	}
};

bool CheckMergeDBType(const char* srcpath)
{
	if(GetDBCrossType() != 0) 
	{
		printf("MergeDBAll error: Dest DB type invalid\n");
		return false;
	}
	
	if(GetStandaloneDBCrossType(srcpath) != 0) 
	{
		printf("MergeDBAll error: Src DB type invalid\n");
		return false;
	}
	
	//检查是否有CROSS_LOCKED状态的role
	bool ret = false;
	std::string src_dir = srcpath;
	{
		DBStandalone* pstandalone = NULL;
		StorageEnv::Uncompressor* uncompressor = NULL;

		CheckCrossLockedQuery q;

		try
		{
			pstandalone = new DBStandalone( (src_dir + "/base").c_str() );
			pstandalone->init();
			uncompressor = new StorageEnv::Uncompressor();

			StorageEnv::AtomTransaction	txn;
			
			try
			{
				q._cross_locked_role_exsit = false;
			
				StorageEnv::Storage::Cursor cursor(&txn, (src_dir + "/base").c_str(), pstandalone, uncompressor);
				cursor.walk( q );

				if(!q._cross_locked_role_exsit) 
				{
					ret = true;
				}
				else
				{
					printf("MergeDBAll error: CrossLocked Role existed in src DB\n");
				}
			}
			catch (...)
			{
				throw;
			}
		}
		catch (...)
		{
			Log::log(LOG_ERR, "CheckCrossLockedRole exception");
		}
		
		pstandalone->checkpoint();
		
		if(pstandalone != NULL) 
		{
			delete pstandalone;
			pstandalone = NULL;
		}

		if(uncompressor != NULL)
		{
			delete uncompressor;
			uncompressor = NULL;
		}
	}
	
	if(!ret) return ret;

	//检查src数据库是否整合过修改后的名字了
	ret = false;
	{
		//先检查rolenamehis这个表是否存在，如果存在的话判断索引为0的地方是否为空
		if( 0 == access((src_dir + "/rolenamehis").c_str(), R_OK) )
		{
			DBStandalone* pstandalone = NULL;
			StorageEnv::Uncompressor* uncompressor = NULL;

			try
			{
				pstandalone = new DBStandalone( (src_dir + "/rolenamehis").c_str() );
				pstandalone->init();
				uncompressor = new StorageEnv::Uncompressor();
				
				StorageEnv::AtomTransaction	txn;
				
				try
				{
					Marshal::OctetsStream key_all, value_all;
					key_all << (int)0; 

					size_t val_len; 
					if( void* val = pstandalone->find( key_all.begin(), key_all.size(), &val_len ) )
					{  
						GNET::Octets dbval = uncompressor->Update(GNET::Octets(val, val_len));
						free(val);
						
						typedef std::map<int, Octets> NameMap;
						NameMap name_map;
						Marshal::OctetsStream(dbval) >> name_map;
						if (0 == name_map.size())	
							ret = true;
						else
							Log::log(LOG_ERR, "srcdb should do syncplayername first!");
					}
					else
					{
						//这个服还没有修改过名字，不需要整合
						ret = true;
					}
				}
				catch (...)
				{
					throw;
				}
			}
			catch (...)
			{
				Log::log(LOG_ERR, "CheckRoleNameHis exception");
			}
			if (pstandalone)
			{
				pstandalone->checkpoint();
				delete pstandalone;
			}
			if (uncompressor)
			{
				delete uncompressor;
			}
		}
	}
	
	if(!ret) return ret;

	//检查dst数据库是否整合过修改后的名字了
	ret = false;
	{
		StorageEnv::Storage * pstorage = StorageEnv::GetStorage("rolenamehis");

		StorageEnv::AtomTransaction	txn;

		try{
			Marshal::OctetsStream key_all, value_all;
			key_all << (int)0; 
			
			if (pstorage->find(key_all, value_all, txn))
			{
				typedef std::map<int, Octets> NameMap;
				NameMap name_map;
				value_all >> name_map;
				if (0 == name_map.size())
					ret = true;
				else
					Log::log(LOG_ERR, "dstdb should do syncplayername first!");
			}
			else
			{
				//这个服没有人改过名字，不需要整合
				ret = true;
			}
		}
		catch( DbException e)
		{
			Log::log(LOG_ERR, "CheckRoleNameHis 2 exception");
		}
	}

	return ret;
}

void ClearRoleCrossData()
{
	try
	{
		StorageEnv::Storage* pstorage = StorageEnv::GetStorage( "base" );
		StorageEnv::AtomTransaction	txn;
		
		for(unsigned int i = 0; i < g_need_clear_crossinfo_list.size(); ++i)
		{
			int roleid = g_need_clear_crossinfo_list[i];
			
			try
			{
				Marshal::OctetsStream os_roleid, os_base;
				os_roleid << roleid; 
				
				if(pstorage->find(os_roleid, os_base, txn))
				{
					GRoleBase base;
					os_base >> base;
					
					base.cross_data.clear();
					pstorage->insert(os_roleid, Marshal::OctetsStream() << base, txn);
				}
			}
			catch ( DbException e ) { throw; }
			catch ( ... )
			{
				DbException ee( DB_OLD_VERSION );
				txn.abort( ee );
				throw ee;
			}
		}
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "ClearRoleCrossData, what=%s\n", e.what() );
	}
	
	StorageEnv::checkpoint();
}

void OutputMergeReport(int zoneid, int srczoneid);

/**
 * 增加跨服功能后，合服完整操作流程
 * 
 * 假设普通服务器甲、乙、丙共享同一个跨服服务器丁。现需要将甲乙服务器合并，甲乙丁三个服务器需要停服，直至合并操作结束再统一开服。丙服务器不需要停服，但是会暂停跨服功能。
 * 
 * 将服务器甲中CROSS_LOCKED状态的玩家数据从跨服数据库中抽取回来，命令行参数：
 * 		./gamedbd.wdb gamesys.conf abstractroles centraldb/dbdata xx(服务器甲的zoneid)
 *
 * 将服务器乙中CROSS_LOCKED状态的玩家数据从跨服数据库中抽取回来，命令行参数：
 *		./gamedbd.wdb gamesys.conf abstractroles centraldb/dbdata yy(服务器乙的zoneid)
 *
 * 将在跨服数据库上，来自甲服务器的角色删除，命令行参数:
 * 		./gamedbd.wdb gamesys.conf delzoneplayers xx(服务器甲的zoneid)
 * 		注: 这是在跨服数据库的gamedbd.wdb所在目录执行的
 *
 * 将在跨服数据库上，来自乙服务器的角色删除，命令行参数:
 * 		./gamedbd.wdb gamesys.conf delzoneplayers yy(服务器乙的zoneid)
 * 		注: 这是在跨服数据库的gamedbd.wdb所在目录执行的
 *
 * 将处理之后的甲乙数据库合并，命令行参数：
 *  	./gamedbd.wdb gamesys.conf mergewdb dbhomewdb/dbdata(若是将甲服务器的数据合并到乙服务器，这是甲数据库的数据路径)
 *		注: 合服的过程中，会将甲乙两个服务器上登陆过跨服角色的跨服信息清空
 */
void MergeDBAll( const char * srcpath, int srczoneid )
{
	std::string data_dir = StorageEnv::get_datadir();

	LoadConfig( );

	// determine map
	if(!CheckMergeDBType(srcpath)) return;
	if(!PrepareLogicuid()) return;
	
	ClearRoleCrossData();

	MergeUser( srcpath );
	MergeUserStore( srcpath );
	MergeRolename( srcpath );
	ResetReferrer();

	MergeFactioninfo( srcpath );
	MergeFactionname( srcpath );
	MergeUserfaction( srcpath );
	MergeFactionfortress( srcpath );
	MergeFactionrelation( srcpath );

	printf( "\nMap Size:\n\tINFO:mapRoleid.size = %d\n\tINFO:mapRolename.size = %d\n\tINFO:mapFactionid.size = %d\n\tINFO:mapFactionname.size = %d\n", g_mapRoleid.size(), g_mapRolename.size(), g_mapFactionid.size(), g_mapFactionname.size() );

	// key改roleid
	MergeBase( srcpath );
	MergeRoleData( srcpath, "status" );
	MergeRoleData( srcpath, "equipment" );
	MergeRoleData( srcpath, "inventory" );
	MergeRoleData( srcpath, "storehouse" );
	MergeRoleData( srcpath, "task" );
	MergeRoleData( srcpath, "waitdel" );
	MergeRoleData( srcpath, "playerprofile" );

	// key改roleid,value改roleid,factionid,name
	MergeMailbox( srcpath );
	MergeMessages( srcpath );
	MergeFriends( srcpath );
	MergeFriendExt( srcpath );
	MergePlayerShop( srcpath );
	// others
	MergeAuction( srcpath );
	MergeShoplog( srcpath );
	MergeSyslog( srcpath );
	MergeWebTrade( srcpath, "webtrade" );
	MergeWebTrade( srcpath, "webtradesold" );
	DelOldWebTradeSold();
	MergeForce( srcpath );
	MergeUniqueData( srcpath );

	StorageEnv::checkpoint();
	StorageEnv::Close();

	system( ("/bin/rm -f " + data_dir + "/order").c_str() );
	system( ("/bin/rm -f " + data_dir + "/city").c_str() );
	system( ("/bin/rm -f " + data_dir + "/gtask").c_str() );
	system( ("/bin/rm -f " + data_dir + "/serverdata").c_str() );
	system( ("/bin/rm -f " + data_dir + "/globalcontrol").c_str() );
	system( ("/bin/rm -f " + data_dir + "/rolenamehis").c_str() );
	system( ("/bin/rm -f " + data_dir + "/kingelection").c_str() );
    system( ("/bin/rm -f " + data_dir + "/mappassword").c_str() );

	StorageEnv::Open();
	StorageEnv::checkpoint( );
	StorageEnv::removeoldlogs( );

	printf( "\nMap Size:\n\tINFO:mapRoleid.size = %d\n\tINFO:mapRolename.size = %d\n\tINFO:mapFactionid.size = %d\n\tINFO:mapFactionname.size = %d\n", g_mapRoleid.size(), g_mapRolename.size(), g_mapFactionid.size(), g_mapFactionname.size() );
	printf( "\nMerge Report:\n\tINFO:server2_usercount = %d\n\tINFO:same_usercount = %d\n\tINFO:server2_rolecount = %d\n\tINFO:same_rolecount = %d\n\tINFO:dup_roleid = %d\n\tINFO:dup_factionid = %d\n\tINFO:dup_rolename = %d\n\tINFO:dup_factionname = %d\n", server2_usercount, same_usercount, server2_rolecount, same_rolecount, dup_roleid, dup_factionid, dup_rolename, dup_factionname );

	Conf* conf=Conf::GetInstance();
	int zoneid = atoi(conf->find("GameDBServer", "zoneid").c_str());
	OutputMergeReport(zoneid, srczoneid);
}

void OutputMergeReport(int zoneid, int srczoneid)
{
	FILE * file = fopen("mergereport", "w");
	if(file == NULL) return;

	fprintf(file, "[General]\n");
	fprintf(file, ";合服过程 将srczoneid服务器合并到zoneid服务器,合并后的服务器使用zoneid\n");
	fprintf(file, "srczoneid\t=\t%d\n",srczoneid);
	fprintf(file, "zoneid\t=\t%d\n",zoneid);
	fprintf(file, "\n");

	fprintf(file, "[RoleIdChanged]\n");
	fprintf(file, ";srczoneid服务器中 改变的角色id列表 oldroleid = newroleid\n");
	std::map<int,int>::iterator it = g_mapRoleid.begin(), ite = g_mapRoleid.end();
	for( ; it!=ite; ++it)
		if(it->first != it->second) 
			fprintf(file,"%d\t=\t%d\n",it->first,it->second);
	fprintf(file, "\n");
	
	fprintf(file, "[FactionIdChanged]\n");
	fprintf(file, ";srczoneid服务器中 改变的帮派id列表 oldfid = newfid\n");
	it = g_mapFactionid.begin(), ite = g_mapFactionid.end();
	for( ; it!=ite; ++it)
		if(it->first != it->second) 
			fprintf(file,"%d\t=\t%d\n",it->first,it->second);
	fprintf(file, "\n");
	
	
	fclose(file);
}

};

#else

namespace CMGDB
{
void MergeDB( const char * srcpath, const char * srcdbname, const char * destdbname )
{
}

void MergeDBAll( const char * srcpath, int srczoneid )
{
}

};

#endif

