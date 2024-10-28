#ifndef __GAMEDB_IF_H__
#define __GAMEDB_IF_H__

//需要与localmacro.h 的CDB_PUT_*保持一致
#define DBMASK_PUT_STOREHOUSE     0x00000001
#define DBMASK_PUT_INVENTORY      0x00000002
#define DBMASK_PUT_TASK           0x00000004
#define DBMASK_PUT_EQUIPMENT      0x00000008
#define DBMASK_PUT_CASH           0x00000010
#define DBMASK_PUT_USERSTORE      0x00000020
#define DBMASK_PUT_ALL            0x0000003F

#define DBMASK_PUT_SYNC_TIMEOUT  (DBMASK_PUT_ALL^(DBMASK_PUT_INVENTORY | DBMASK_PUT_EQUIPMENT))

namespace GNET
{
	class GRoleDetail;
	class GRoleInventory;
	template <class T> class RpcDataVector;
	//class RpcDataVector<GRoleInventory>;
//	typedef RpcDataVector<GRoleInventory>	GRoleInventoryVector;
};

enum
{
//在 ../gamedbd/dbwebtradeprepost.hrp，../gamedbd/dbwebtradesold.hrp 两个文件中有相同的定义，请保持一致
//注意: 增加新的数据时需要对寻宝网角色交易、角色跨服中角色数据拷贝的代码进行修改
//角色交易(gamedbd/dbwebtradesold.hrp)
//角色跨服(gamedbd/abstractplayers.cpp AbstractPlayerData())
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

namespace GDB
{

struct base_info
{
	unsigned int 	id;
	unsigned int 	userid;
	int 		race;		//种族
	int 		cls;		//职业
	bool 		gender;		//性别
	bool		trashbox_active;
	bool		cash_active;
	bool		userstore_active; //账号仓库是否需要保存
	int 		level;
	int		sec_level;
	int 		exp;
	int 		sp;
	int		pp;
	int 		hp;
	int		mp;
	float 		posx;
	float 		posy;
	float 		posz;
	int		worldtag;
	int		money;
	int 		custom_crc;
	int 		invader_state;
	int 		invader_time;
	int 		pariah_time;
	unsigned int 	factionid;
	int 		factionrole;
	int 		reputation;
	int		dbltime_expire;
	int		dbltime_mode;
	int		dbltime_begin;
	int		dbltime_used;
	int		dbltime_max;
	int		time_used;
	size_t		trash_money;
	int		create_time;
	int		lastlogin_time;
	short		storesize;
	short		storesize1;
	short		storesize2;
	short		storesize3;
	short		bagsize;
	int		timestamp;
	int             cash;
	int             cash_add; //累计充值金额
	int             cash_total;
	int             cash_used;
	int             cash_delta;
	int             cash_serial;
	unsigned int	spouse;
	int		bonus_add;
	int		bonus_reward;
	int		bonus_used;
	int 		referrer;
	int 		userstoresize;  //账号仓库可操作格子数
	int		userstoremoney; //账号仓库中的money
	int mall_consumption; //消费值
	int	country_id;				//国家id
	int	country_expire_time;	//国家过期时间
	int is_king;
	int king_expire_time;
	int	src_zoneid;

	struct{
    	int64_t  sn;
	    char     state;
	    unsigned cost;
    	unsigned lots;
	    unsigned itemcount;
    	int 	 itemtype;
		int		 itemexpire;
	} touch_trade;

	struct{
		int update_time;
		int month_calendar;
		int curr_year_data;
		int last_year_data;
        char awarded_times;
        char late_signin_times;
		short reserved;
	} daily_signin;

	struct{
		char	state; 
		int		type;
		int		parenttype;
		char	cardnumber[20];
	} giftcard_redeem;

	int leadership;

	struct{
		int contrib;
		int cost;
	}world_contribution;

	struct{
		unsigned char level;
		int  exp;
	} astrolabe_extern;
};

struct ivec
{
	const void * data;
	unsigned int size;
};

struct itemdata
{
	unsigned int id;
	int index;
	int count;
	int max_count;
	int guid1;
	int guid2;
	int mask;
	int proctype;
	int expire_date;
	const void * data;
	unsigned int size;
};


struct  itemlist
{
	itemdata * list;
	size_t count;
};

struct shoplog          
{       
	int order_id;   
	int item_id;    
	int expire;     
	int item_count; 
	int order_count;
	int cash_need;  
	int time;       
	int guid1;      
	int guid2;      
};      
        
struct  loglist         
{       
	shoplog * list; 
	size_t count;   
};     

struct  pet
{
	size_t index;
	ivec data;
};

struct  petcorral
{
	size_t capacity;
	size_t count;
	pet * list;
};

struct faction_relation_data
{
	size_t alliance_count;
	int * alliance_list;
	size_t hostile_count;
	int * hostile_list;
};

struct forcedata
{
	int force_id;
	int reputation;
	int contribution;
};

struct forcedata_list
{
	int cur_force_id;
	size_t count;
	forcedata * list;
};

struct meridian_data
{
	int meridian_level;
	int lifegate_times;
	int deathgate_times;
	int	free_refine_times;  
	int paid_refine_times;
	int player_login_time; //当天的0点
	int continu_login_days; //连续上线天数
	int trigrams_map[3];
};

struct reincarnation_record
{
	int level;
	int timestamp;
	int exp;
};

struct reincarnation_data
{
	int tome_exp;
	char tome_active;
	size_t count;
	reincarnation_record * records; 
};

struct realm_data
{
	int level;
	int exp;
	int reserved1;
	int reserved2;
};

struct vecdata
{
	ivec user_name;		//这个数据不会存盘
	ivec custom_status;
	ivec filter_data;
	ivec charactermode;
	ivec instancekeylist;
	ivec property;
	itemlist inventory;
	itemlist equipment;
	itemlist task_inventory;
	itemlist trash_box;
	itemlist trash_box1;
	itemlist trash_box2;
	itemlist trash_box3;
	itemlist user_store;
	ivec skill_data;
	ivec task_data;
	ivec finished_task_data;
	ivec finished_time_task_data;
	ivec var_data;
	ivec trashbox_passwd;
	ivec waypoint_list;
	ivec coolingtime;
	ivec npc_relation;
	petcorral pets;
	ivec dbltime_data;		
	ivec addiction_data;		
	loglist logs;
	ivec task_counter;		
	ivec multi_exp_ctrl;
	ivec storage_task;
	ivec faction_contrib;
	faction_relation_data faction_relation;
	forcedata_list force_data;
	ivec online_award;
	ivec profit_time_data;
	meridian_data meridian;
	ivec title_data;
	reincarnation_data reincarnation;
	realm_data	realm;
	ivec generalcard_collection;
	ivec fatering_data;
	ivec clock_data;
	ivec rand_mall_data;
};


class Result
{
public:
	virtual void OnTimeOut() = 0;
	virtual void OnFailed() = 0;
	virtual void OnGetRole(int id,const base_info * pInfo, const vecdata * data,const GNET::GRoleDetail * pRole){}
	virtual void OnPutRole(int retcode){}
	virtual void OnPutMoneyInventory(int retcode) {}
	virtual void OnGetMoneyInventory(size_t money, const itemlist & list, int timestamp, int getmask) {}
	virtual void OnGetCashTotal(int cash_total){}
	virtual ~Result(){}
};

bool init_gamedb();
//$$$超时60秒
bool put_role(int id,const base_info* pInfo,const vecdata* data,Result *callback=NULL,int priority=0,int mask=DBMASK_PUT_ALL);
bool get_role(int id, Result * callback);

bool put_money_inventory(int id, size_t money, itemlist & list,Result * callback);
bool get_money_inventory(int id, Result * callback, int getmask=0);

bool get_cash_total(int userid, Result * callback);

//size_t convert_item(const GRoleInventoryVector & ivec, itemdata * list , size_t size);
size_t convert_item(const GNET::RpcDataVector<GNET::GRoleInventory> & ivec,  itemdata * list , size_t size);
bool set_couple(int id1, int id2, int op);

void itemlist_to_inventory(GNET::RpcDataVector<GNET::GRoleInventory> & ivec, const itemlist & list);


//需要与localmacro.h 的CDBMASK_SERVERDATA_*保持一致
#define DBMASK_SERVERDATA_WEDDING		0x00000001
#define DBMASK_SERVERDATA_DPSRANK		0x00000002
#define DBMASK_SERVERDATA_ALL			0x00000003

struct serverdata
{
	int world_tag;
	ivec wedding_data;
	ivec dpsrank_data;
};

class ServerDataResult
{
public:
	virtual void OnTimeOut() = 0;
	virtual void OnFailed() = 0;
	virtual void OnGetServerData(int world_tag, const serverdata * data, int mask){}
	virtual void OnPutServerData(){}
	virtual ~ServerDataResult(){} 
};

bool put_serverdata(int world_tag, const serverdata* data, ServerDataResult * callback=NULL, int priority=0, int mask=DBMASK_SERVERDATA_ALL);
bool get_serverdata(int world_tag, ServerDataResult * callback, int mask=DBMASK_SERVERDATA_ALL);

class CopyRoleResult
{
public:
    virtual void OnTimeOut() = 0;
    virtual void OnFailed() = 0;
    virtual void OnSucceed() { }
    virtual ~CopyRoleResult() { }
};

bool copy_role(int src_roleid, int dst_roleid, CopyRoleResult* callback);
};

#endif


