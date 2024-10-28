#include <stdio.h>
#include "conf.h"
#include "log.h"
#include <ios>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <string>

#include "roleid"
#include "groleinventory"
#include "grolepocket"
#include "grolestorehouse"
#include "gauctionitem"
#include "gauctiondetail"
#include "storage.h"
#include "gmail"
#include "grolebase"
#include "stocklog"
#include "user"
#include "guserstorehouse"
#include "userid"
#include "gmailbox"
#include "localmacro.h"
#include "gsysauctioncash"
#include "gwebtradedetail"
#include "crossinfodata"
#include "pshopdetail"
#include "grewardstore" 
#include "gcashvipinfo"
namespace GNET
{

#define ROLEIDMIN(x) x
#define ROLEIDMAX(x) (x | 0x0000000f)

	std::string config_file = "cashstat.conf";
	std::string dbhome = "dbhomewdb";
	std::string shopdata_file;
	bool loaded = false;
#pragma pack(push, GSHOP_ITEM_PACK, 1)
	typedef struct _GSHOP_ITEM
	{
		int			local_id;	// id of this shop item, used only for localization purpose
		int			main_type;	// index into the main type array
		int			sub_type;	// index into the sub type arrray

		char			icon[128];	// icon file path

		unsigned int		id;		// object id of this item
		unsigned int		num;		// number of objects in this item

		struct {
			unsigned int	price;		// price of this item
			unsigned int	until_time;	// valid until time, 0 means not set
			unsigned int	time;		// time of duration, 0 means forever
		} list[4];

		unsigned int		props;		// mask of all props, currently from low bit to high bit: 新品、推荐品、促销品
		unsigned short		desc[512];	// simple description
		unsigned short 		szName[32];	// name of this item to show

	} GSHOP_ITEM;
#pragma pack(pop, GSHOP_ITEM_PACK)

	typedef std::map<unsigned int /*itemid*/, unsigned int /*count*/> TRoleItems;
	typedef std::map<unsigned int /*itemid*/, unsigned int /*count*/> ItemCounter;
	struct TRoleData
	{
		int remote_roleid;
		int src_zoneid;
		bool cross_locked;
		TRoleItems items;
		TRoleData():remote_roleid(0),src_zoneid(0),cross_locked(false) { }
	};

	typedef std::map<unsigned int, TRoleData> RoleMap;
	typedef std::map<int, int> ItemSet;

	struct UserData
	{
		int logintime;
		unsigned int logicuid;
		unsigned int rolelist;
		unsigned int cash_add;
		unsigned int cash_buy;
		unsigned int cash_sell;
		unsigned int cash_used;
		unsigned int cash_used_2;
		bool obsolete;
		int          cash_vip_level;
		unsigned int cash_vip_score_add;
		unsigned int cash_vip_score_daily_reduce;
		unsigned int cash_vip_score_consume;
		TRoleItems userstore; /*存储账号仓库中的人民币物品*/
		UserData():logintime(0),logicuid(0),cash_add(0),cash_buy(0),cash_sell(0),cash_used(0),cash_used_2(0),obsolete(0),cash_vip_level(0),cash_vip_score_add(0),cash_vip_score_daily_reduce(0),cash_vip_score_consume(0) { }
	};
	typedef std::map<unsigned int, UserData>  UserMap;


	static void TravelTable(const char *table_name, StorageEnv::IQuery &q)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage(table_name);
			StorageEnv::AtomTransaction txn;
			try
			{
				StorageEnv::Storage::Cursor cursor = pstorage->cursor( txn );
				cursor.browse( q );
			}
			catch (DbException e) { throw; }
			catch (...)
			{
				DbException ee( DB_OLD_VERSION );
				txn.abort( ee );
				throw ee;
			}
		}
		catch ( DbException e )
		{
			Log::log(LOG_ERR, "err occur when walking %s\n", table_name);
			return;
		}
	}

	struct TDataBase
	{
		UserMap user_map;
		RoleMap role_map;
		ItemCounter item_counter;

		struct PocketQuery : public StorageEnv::IQuery
		{   
			TDataBase *tdatabase;
			bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
			{
				Marshal::OctetsStream key_os(key);
				Marshal::OctetsStream value_os(value);

				RoleId      roleid;
				GRolePocket pocket;
				try
				{
					key_os >> roleid;
					value_os >> pocket;

					if (pocket.items.size() == 0)
						return true;

					RoleMap::iterator it = tdatabase->role_map.end();

					GRoleInventoryVector::const_iterator cit = pocket.items.begin();
					for (; cit != pocket.items.end(); ++ cit)
						tdatabase->AddRoleItem(it, roleid.id, cit->id, cit->count);

				} catch ( Marshal::Exception & ) {
					Log::log( LOG_ERR, "PocketQuery, error unmarshal");
					return true;
				}
				return true;
			}
		};

		struct EquipmentQuery : public StorageEnv::IQuery
		{
			TDataBase *tdatabase;
			bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
			{
				Marshal::OctetsStream key_os(key);
				Marshal::OctetsStream value_os(value);

				RoleId      roleid;
				GRoleInventoryVector equipments;
				try
				{
					key_os >> roleid;
					value_os >> equipments;

					if (equipments.size() == 0)
						return true;

					RoleMap::iterator it = tdatabase->role_map.end();

					GRoleInventoryVector::const_iterator cit = equipments.begin();
					for (; cit != equipments.end(); ++ cit)
						tdatabase->AddRoleItem(it, roleid.id, cit->id, cit->count);

				} catch ( Marshal::Exception & ) {
					Log::log( LOG_ERR, "EquipmentQuery, error unmarshal");
					return true;
				}
				return true;
			}
		}; 

		struct StorehouseQuery : public StorageEnv::IQuery
		{   
			TDataBase *tdatabase;
			bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
			{
				Marshal::OctetsStream key_os(key);
				Marshal::OctetsStream value_os(value);

				RoleId      roleid;
				GRoleStorehouse storehouse;
				try
				{
					key_os >> roleid;
					value_os >> storehouse;

					RoleMap::iterator it = tdatabase->role_map.end();
					GRoleInventoryVector::const_iterator cit = storehouse.items.begin();
					for (; cit != storehouse.items.end(); ++ cit)
						tdatabase->AddRoleItem(it, roleid.id, cit->id, cit->count);
					cit = storehouse.dress.begin();
					for (; cit != storehouse.dress.end(); ++cit)
						tdatabase->AddRoleItem(it, roleid.id, cit->id, cit->count);
					cit = storehouse.material.begin();
					for (; cit != storehouse.material.end(); ++cit)
						tdatabase->AddRoleItem(it, roleid.id, cit->id, cit->count);
					cit = storehouse.generalcard.begin();
					for (; cit != storehouse.generalcard.end(); ++cit)
						tdatabase->AddRoleItem(it, roleid.id, cit->id, cit->count);
				} catch ( Marshal::Exception & ) {
					Log::log( LOG_ERR, "StorehouseQuery, error unmarshal.");
					return true;
				}
				return true;
			}
		};

		struct MailQuery : public StorageEnv::IQuery
		{   
			TDataBase *tdatabase;
			bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
			{
				Marshal::OctetsStream key_os(key);
				Marshal::OctetsStream value_os(value);

				RoleId roleid;
				GMailBox mailbox;
				try
				{
					key_os >> roleid;
					value_os >> mailbox;

					RoleMap::iterator it = tdatabase->role_map.end();
					GMailVector::iterator mit = mailbox.mails.begin();
					for (; mit != mailbox.mails.end(); ++mit)
					{
						if(mit->header.attribute & (1 << _MA_ATTACH_OBJ))
							tdatabase->AddRoleItem(it, roleid.id, mit->attach_obj.id, mit->attach_obj.count);
					}

				} catch ( Marshal::Exception & ) {
					Log::log( LOG_ERR, "MailQuery, error unmarshal.");
					return true;
				}
				return true;
			}
		};

		struct BaseQuery : public StorageEnv::IQuery
		{
			TDataBase *tdatabase;
			bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
			{
				Marshal::OctetsStream key_os;
				key_os = key;
				Marshal::OctetsStream value_os;
				value_os = value;

				RoleId roleid;
				GRoleBase base;
				try
				{
					key_os >> roleid;
					value_os >> base;
					int userid = base.userid;
					if(!userid)
						userid = LOGICUID(roleid.id);
					UserMap::iterator iu = tdatabase->user_map.find(userid);
					if (iu != tdatabase->user_map.end()) 
					{
						if(base.lastlogin_time>iu->second.logintime)
							iu->second.logintime = base.lastlogin_time;
					}
					tdatabase->ReadBase(roleid.id, base);
				} catch ( Marshal::Exception & ) {
					Log::log( LOG_ERR, "BaseQuery, error unmarshal.");
					return true;
				}
				return true;
			}
		};

		struct UserQuery : public StorageEnv::IQuery
		{   
			TDataBase *tdatabase;
			bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
			{
				Marshal::OctetsStream key_os;
				key_os = key;
				Marshal::OctetsStream value_os;
				value_os = value;

				UserID userid; 
				User user;
				try
				{
					key_os >> userid;
					value_os >> user;

					tdatabase->ReadUser(userid.id, user);

				} catch ( Marshal::Exception & ) {
					Log::log( LOG_ERR, "UserQuery, error unmarshal.");
					return true;
				}
				return true;
			}
		};

		struct UserStoreQuery : public StorageEnv::IQuery
		{   
			TDataBase *tdatabase;
			bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
			{
				Marshal::OctetsStream key_os;
				key_os = key;
				Marshal::OctetsStream value_os;
				value_os = value;

				UserID userid; 
				GUserStorehouse userstorehouse;
				try
				{
					key_os >> userid;
					value_os >> userstorehouse;

					tdatabase->ReadUserStore(userid.id, userstorehouse);

				} catch ( Marshal::Exception & ) {
					Log::log( LOG_ERR, "UserStoreQuery, error unmarshal.");
					return true;
				}
				return true;
			}
		};

		struct AuctionQuery : public StorageEnv::IQuery
		{
			TDataBase *tdatabase;
			bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
			{
				Marshal::OctetsStream value_os(value);

				GAuctionDetail detail;
				try
				{
					value_os >> detail;
					RoleMap::iterator it = tdatabase->role_map.end();
					tdatabase->AddRoleItem(it, detail.info.seller, detail.item.id, detail.item.count);
				} catch ( Marshal::Exception & ) {
					Log::log( LOG_ERR, "AuctionQuery, error unmarshal.");
					return true;
				}
				return true;
			}
		};

		struct WebTradeQuery : public StorageEnv::IQuery
		{
			TDataBase *tdatabase;
			bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
			{
				Marshal::OctetsStream key_os(key), value_os(value);

				int64_t sn;
				GWebTradeDetail detail;
				try
				{
					key_os >> sn;
					if(sn == 0) 
						return true;
					value_os >> detail;
					RoleMap::iterator it = tdatabase->role_map.end();
					if(detail.info.posttype == 2)
						tdatabase->AddRoleItem(it, detail.info.seller_roleid, detail.item.id, detail.item.count);
				} catch ( Marshal::Exception & ) {
					Log::log( LOG_ERR, "WebTradeQuery, error unmarshal.");
					return true;
				}
				return true;
			}
		};
	
		struct PlayerShopQuery : public StorageEnv::IQuery
		{
			TDataBase *tdatabase;
			bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
			{
				Marshal::OctetsStream key_os(key), value_os(value);

				PShopDetail shop;
				RoleId roleid;
				try
				{
					key_os >> roleid;
					value_os >> shop;
					RoleMap::iterator it = tdatabase->role_map.end();
					GRoleInventoryVector::const_iterator cit = shop.yinpiao.begin();
					for (; cit != shop.yinpiao.end(); ++ cit)
						tdatabase->AddRoleItem(it, roleid.id, cit->id, cit->count);
					cit = shop.store.begin();
					for (; cit != shop.store.end(); ++cit)
						tdatabase->AddRoleItem(it, roleid.id, cit->id, cit->count);

					PShopItemVector::const_iterator cpit = shop.slist.begin();
					for (; cpit != shop.slist.end(); ++cpit)
						tdatabase->AddRoleItem(it, roleid.id, cpit->item.id, cpit->item.count);

				} catch ( Marshal::Exception & ) {
					Log::log( LOG_ERR, "PlayerShopQuery, error unmarshal.");
					return true;
				}
				return true;
			}
		};
	
		bool CountItem(int id, int count)
		{
			ItemCounter::iterator it = item_counter.find(id);
			if (it != item_counter.end())
			{
				it->second += count;
				return true;
			}
			return false;
		}

		void AddRoleItem(RoleMap::iterator & iter, int rid, int id, int count)
		{
			if (CountItem(id, count))
			{
				if (iter == role_map.end())
				{
					iter = role_map.find(rid);
					if (iter == role_map.end())
						iter = role_map.insert(std::make_pair(rid, TRoleData())).first;
				}	
				iter->second.items[id] += count;
			}
		}
		// load shop data from gshop.data 
		bool LoadShopData()
		{
			LoadConfItem();
			std::ifstream ifs(shopdata_file.c_str(), std::ios::in | std::ios::binary);
			if (!ifs) return  false;

			int size = sizeof(GSHOP_ITEM);
			GSHOP_ITEM data;
			try
			{
				int timestamp, nCount;
				ifs.read((char*)&timestamp, 4);
				ifs.read((char*)&nCount, 4);

				if (nCount < 0 || nCount > 65535) return false;
				printf("count = %d\n", nCount);
				for(int i=0; i<nCount; i++)
				{
					ifs.read((char*)&data, size);
					item_counter.insert(std::make_pair(data.id, 0));
				}
			}
			catch(...) { }
			return true;
		}

		// update shop data to config file
		void UpdateConfigFile()
		{
			// read & modify config file 
			std::ifstream ifs(config_file.c_str(), std::ios::in);
			if (!ifs)
			{ 
				std::cout << "can't update config file, " << config_file << " open failed" << std::endl;
				return ;
			}
			std::ostringstream os;
			std::string line;
			bool inflag = false;
			while (std::getline(ifs, line))
			{
				if (!inflag)
				{
					if (line == "[items]")
					{
						inflag = true;
						os << line << "\n";
						ItemCounter::iterator it = item_counter.begin();	
						for (; it != item_counter.end(); ++ it)
							os << it->first << " = 1\n";
						continue;	
					}

					os << line << "\n";
				}
				else
					if (line[0] == '[') 
					{
						inflag = false;	
						os << line << "\n";
					}

			}
			ifs.close();
			// write config file
			std::ofstream ofs(config_file.c_str(), std::ios::out | std::ios::trunc);
			if (!ofs)
			{ 
				std::cout << "can't update config file, " << config_file << " open failed" << std::endl;
				return ;
			}
			ofs << os.str();
			ofs.close();
		}

		void LoadConfItem()
		{
			// combine shop data	
			Conf * conf = Conf::GetInstance();
			std::vector<std::string> keys;
			conf->getkeys("items", keys);
			std::vector<std::string>::iterator it = keys.begin();
			for (; it != keys.end(); ++ it)
			{
				if (atoi(conf->find("items", *it).c_str()) == 1)
					item_counter.insert(std::make_pair(atoi(it->c_str()), 0));	
			}	
		}

		void Load()
		{
			LoadConfItem();
			UserQuery uq;
			uq.tdatabase = this;
			TravelTable("user", uq);

			UserStoreQuery ustoreq;
			ustoreq.tdatabase = this;
			TravelTable("userstore", ustoreq);

			PocketQuery qp;
			qp.tdatabase = this;
			TravelTable("inventory", qp);

			EquipmentQuery ep;
			ep.tdatabase = this;
			TravelTable("equipment", ep);

			StorehouseQuery qs;
			qs.tdatabase = this;
			TravelTable("storehouse", qs);

			MailQuery mq;
			mq.tdatabase = this;
			TravelTable("mailbox", mq);

			AuctionQuery aq;
			aq.tdatabase = this;
			TravelTable("auction", aq);

			BaseQuery bq;
			bq.tdatabase = this;
			TravelTable("base", bq);

			WebTradeQuery wq;
			wq.tdatabase = this;
			TravelTable("webtrade", wq);

			PlayerShopQuery psq;
			psq.tdatabase = this;
			TravelTable("playershop", psq);
		}
		void PrintHeader(int userid, int roleid, UserData& data, const TRoleData* roledata)
		{
			std::cout << roleid << "," << userid << "," << data.logintime << "," << data.obsolete;
			if (roledata != NULL)
				std::cout << "," << roledata->remote_roleid << "," << roledata->src_zoneid <<
						"," << (roledata->cross_locked ? 1 : 0);
			else
				std::cout << ",0,0,0";
			std::cout << ",add:" << data.cash_add << ";buy:" << data.cash_buy << ";sell:"
					<< data.cash_sell << ";used:" << data.cash_used + data.cash_used_2 << ";";
			std::cout << "cash_vip_level:" << data.cash_vip_level << ";cash_vip_score_add:" << data.cash_vip_score_add << ";cash_vip_score_reduce:" << data.cash_vip_score_daily_reduce << ";cash_vip_score_consume:" << data.cash_vip_score_consume << ";";
		}

		void Dump()
		{
			time_t now = time(NULL);
			for(UserMap::iterator iu=user_map.begin();iu!=user_map.end();++iu)
			{
				UserData& data = iu->second;
				if(data.logintime && now-data.logintime>365*24*3600)
				{
					// 超过365天不登陆的账户，设置360标记
					data.obsolete = true;
				}
				int userid = iu->first;
				int roleid = data.logicuid;
				int lastrole = 0;
				const TRoleData* lastroledata = NULL;
				bool output = false;
				for(int i=1;i<0x10000;i<<=1,roleid++)
				{
					if((i&data.rolelist)==0)
						continue;
					lastrole = roleid;
					RoleMap::const_iterator it = role_map.find(roleid);
					if(it==role_map.end())
					{
						lastroledata = NULL;
						continue;
					}
					else
						lastroledata = &(it->second);

					if(it->second.items.size() == 0)
						continue;

					PrintHeader(userid,roleid,data,&(it->second));
					TRoleItems::const_iterator cit = it->second.items.begin();
					for (; cit != it->second.items.end(); ++ cit)
						std::cout << cit->first << ":" << cit->second << ";";
					std::cout << "\r\n";
					output = true;
				}
				//如果账号仓库包含人民币物品 一定要输出
				if(data.userstore.size() != 0)
				{
					PrintHeader(userid,0,data,NULL);
					TRoleItems::const_iterator cit = data.userstore.begin();
					for (; cit != data.userstore.end(); ++cit)
						std::cout << cit->first << ":" << cit->second << ";";
					std::cout << "\r\n";
					output = true;
				}
				if (!output && (data.cash_add||data.cash_buy||data.cash_sell||data.cash_used||data.cash_used_2))
				{
					PrintHeader(userid,lastrole,data,lastroledata);
					std::cout << "\r\n";
				}
			}
		}

		void ReadBase(int roleid, const GRoleBase& base)
		{
			RoleMap::iterator it = role_map.find(roleid);
			if(it == role_map.end())
				it = role_map.insert(std::make_pair(roleid, TRoleData())).first;

			TRoleData& roledata = it->second;
			roledata.cross_locked = (base.status == _ROLE_STATUS_CROSS_LOCKED || base.status == _ROLE_STATUS_CROSS_RECYCLE);
			
			if (base.cross_data.size() > 0)
			{
				CrossInfoData cross_info;
				Marshal::OctetsStream(base.cross_data) >> cross_info;
				roledata.remote_roleid = cross_info.remote_roleid;
				roledata.src_zoneid = cross_info.src_zoneid;
			}
			else
			{
				roledata.remote_roleid = 0;
				roledata.src_zoneid = 0;
			}
		}

		void ReadUser(int userid, const User & user)
		{
			UserData data;
			data.logicuid = user.logicuid;
			data.rolelist = user.rolelist;
			data.cash_add = user.cash_add;
			data.cash_buy = user.cash_buy;
			data.cash_sell = user.cash_sell;
			data.cash_used = user.cash_used;
			GSysAuctionCash sa_cash;
			if(user.cash_sysauction.size())
				Marshal::OctetsStream(user.cash_sysauction)>>sa_cash;
			data.cash_used_2 = sa_cash.cash_used_2;
			if(user.consume_reward.size())
			{
				GRewardStore reward_store;
				Marshal::OctetsStream os_reward(user.consume_reward);
				os_reward >> reward_store;
				if(reward_store.cash_vip_info.size())
				{
					GCashVipInfo vipinfo;
					Marshal::OctetsStream os_vip_info(reward_store.cash_vip_info);
					os_vip_info >> vipinfo;
					data.cash_vip_level              = vipinfo.cash_vip_level;
					data.cash_vip_score_add          = vipinfo.score_add;
					data.cash_vip_score_daily_reduce = vipinfo.score_cost;
					data.cash_vip_score_consume      = vipinfo.score_consume;
				}
			}
			user_map[userid] = data;
		}
		void ReadUserStore(int userid, const GUserStorehouse & store)
		{
			UserMap::iterator iu=user_map.find(userid);
			if (iu == user_map.end())
				printf("Err: ReadUserStore can not find user %d in usermap", userid);
			else
			{
				GRoleInventoryVector::const_iterator cit = store.items.begin();
				for (; cit != store.items.end(); ++cit)
				{
					if (CountItem(cit->id, cit->count))
						iu->second.userstore[cit->id] += cit->count;
				}
			}
		}

		void DumpTotal()
		{
			std::ofstream ofs("total.txt");
			if (!ofs)
			{
				printf("Err: output file total.txt open failed\n" );
				return;
			}

			ItemCounter::const_iterator it = item_counter.begin();
			for (; it != item_counter.end(); ++ it)
			{
				ofs << it->first << ":" << it->second << "\r\n";
			}
			ofs.close();
		}
	};

}; // namespace GNET

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " conf-file [dbhome] " << std::endl;
		exit(-1);
	}

	if (argv[1][0] != '-')
	{
		if (access(argv[1], R_OK) == -1 )
		{
			std::cerr << "Usage: " << argv[0] << " conf-file [dbhome] " << std::endl;
			exit(-1);
		}
		config_file = argv[1];

		if (argc >= 3)
		{
			if (-1 == access(argv[2], R_OK))
			{
				std::cerr << "Bad dbhome dir" << std::endl;
				exit(-1);
			}
			dbhome = argv[2];
		}

		if (argc >= 4)
		{
			if (-1 == access(argv[3], R_OK))
			{
				std::cerr << "Bad file : gshop.data" << std::endl;
				exit(-1);
			}
			shopdata_file = argv[3];
		}
	}
	else
	{
		int opt = getopt(argc, argv, "c:D:g:h");
		while( opt != -1 )
		{
			switch( opt )
			{
				case 'c':
					config_file = optarg; /* true */
					break;

				case 'D':
					dbhome = optarg;
					break;

				case 'g':
					shopdata_file = optarg;
					break;
				case 'h':
				default:
					std::cout << "Usage: " << argv[0] << "-c config_file -D dbhome_dir -g gshop.data\r\n";
					exit(0);
			}
			opt = getopt(argc, argv, "c:D:g:h");
		}
	}
	Conf *conf = Conf::GetInstance(config_file.c_str());
#ifdef USE_WDB
	conf->put("storagewdb", "homedir", dbhome);
#else
	conf->put("storage", "homedir", dbhome);
#endif
	if (!shopdata_file.empty())
	{
		TDataBase tdatabase;
		if (!tdatabase.LoadShopData())
		{
			std::cout << "Load shopdata file failed\n";
			exit(-1);
		}
		tdatabase.UpdateConfigFile();
		exit(0);
	}

	Log::setprogname(argv[0]);
	bool r = StorageEnv::Open();
	if (r)
	{
		TDataBase tdatabase;
		tdatabase.Load();
		tdatabase.Dump();
	}
	else
		std::cout << "Err: open error" << std::endl;
	return 0;
}


