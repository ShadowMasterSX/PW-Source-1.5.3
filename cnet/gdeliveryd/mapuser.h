#ifndef __GNET_MAPUSER_H
#define __GNET_MAPUSER_H

#include <map>
#include <set>
#include <vector>

#include "maperaser.h"
#include "thread.h"
#include "groleforbid"
#include "grolebase"
#include "ggroupinfo"
#include "gfriendinfo"
#include "hashstring.h"
#include "gpair"
#include "localmacro.h"
#include "uidconverter.h"
#include "gfriendextinfo"
#include "gsendaumailrecord"
#include "crossinfodata"
#include "roleinfo"

#define ROLELIST_DEFAULT	0x80000000
#define MAX_ROLE_COUNT		16

namespace GNET
{

class AddictionControl;
enum AUTOLOCK_KEY
{
	LOCKTIME_NOW = 1,
	LOCKTIME_NEW = 2,
	LOCKSET_TIME = 3,
	LOCKSET_TIME_GM = 4,
	LOCKTIME_GM = 5, // GM永久锁的有效期，超过这时间后，GM永久锁不再起作用。-1代表永远不失效。
};
class PairSet
{
	std::vector<GPair> list;
public:
	PairSet() {}
	PairSet(std::vector<GPair>& v) : list(v) {}
	PairSet& operator = (const std::vector<GPair>& v)
	{
		list = v;
		return *this;
	}
	int GetValue(int key)
	{
		for(std::vector<GPair>::iterator it=list.begin();it!=list.end();++it)
		{
			if(it->key==key)
				return it->value;
		}
		return -1;
	}
	void SetValue(int key, int value)
	{
		for(std::vector<GPair>::iterator it=list.begin();it!=list.end();++it)
		{
			if(it->key==key)
			{
				it->value = value;
				return;
			}
		}
		list.push_back(GPair(key,value));
	}
	std::vector<GPair>& GetList()
	{
		return list;
	}
};

class PlayerInfo;
class PlayerLogout;
class UserInfo
{
public:
	struct point_t 
	{
		float x;
		float y;
		float z;
		point_t() : x(0.f),y(0.f),z(0.f) { }
		point_t(float _x,float _y,float _z) : x(_x),y(_y),z(_z) { }
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
public:
	int userid;
	Octets account;//登陆账号
	int logicuid;
	int roleid;
	RoleList rolelist;
	unsigned int linksid;
	unsigned int localsid;
	int status;
	int gmstatus;
	int linkid;
	int gameid;
	int ip;							//本次账号登陆ip
	int rewardtype;
	int rewardtype2;
	int rewarddata;
	int switch_gsid; 
	ByteVector privileges;
	AddictionControl*  acstate;
	time_t             actime;
	time_t             logintime;	//本次账号登陆时间
	time_t             lastlogin; 	//上次角色登陆时间
	int	last_login_time;			//上次账号登陆时间
	int	last_login_ip;				//上次账号登陆ip
	PairSet            autolock;
	PlayerInfo*        role;
	int worldtag[16];
	int create_time[16];
	point_t role_pos[16];
	char role_status[16];
	//玩家的推广链接中记录的是roleid，实际使用的是userid
	int real_referrer; //上线的userid
//	int referrer_roleid; //发起推广的上线的roleid
	int au_suggest_districtid; //上线所在区号
	int au_suggest_referrer; //au发来的推广上线 的 roleid
	bool is_phone;	//是否使用手机进行登录
	
	std::set<int> activated_merchants; // 已开通快捷支付的商家

	//cross server related
	Octets iseckey;
	Octets oseckey;
    Octets rand_key; //临时缓存玩家跨服登录使用的随机key
	int src_zoneid; //跨服中玩家来自的zoneid
	CrossInfoData cross_info[16];
	//cross server end

	UserInfo(int uid,const Octets & uname, unsigned int sid,unsigned int lsid,int st)
		: userid(uid), account(uname), linksid(sid),localsid(lsid),status(st)
	{
		for(int i=0;i<16;i++)
		{
			worldtag[i]    = _WORLDTAG_INVALID;
			create_time[i] = 0;
			role_status[i] = 0;
		}
		ip = 0;
		acstate = 0;
		actime  = 0;
		logintime  = 0;
		lastlogin  = 0;
		last_login_time = 0;
		last_login_ip = 0;
		linkid = 0;
		switch_gsid = _GAMESERVER_ID_INVALID;
		gameid  = _GAMESERVER_ID_INVALID;
		role = 0;
		gmstatus = 0;
		real_referrer = 0;
		//referrer_roleid = 0;
		au_suggest_districtid = 0;
		au_suggest_referrer = 0;
		is_phone = false;

		//cross server related
		src_zoneid = 0;
	}
	UserInfo() 
	{
		for(int i=0;i<16;i++)
		{
			worldtag[i]    = _WORLDTAG_INVALID;
			create_time[i] = 0;
			role_status[i] = 0;
		}
		ip = 0;
		acstate = 0;
		actime  = 0;
		logintime  = 0;
		lastlogin  = 0;
		last_login_time = 0;
		last_login_ip = 0;
		linkid = 0;
		switch_gsid = _GAMESERVER_ID_INVALID;
		gameid  = _GAMESERVER_ID_INVALID;
		role = 0;
		real_referrer = 0;
		//referrer_roleid = 0;
		au_suggest_districtid = 0;
		au_suggest_referrer = 0;
		is_phone = false;
		
		//cross server related
		src_zoneid = 0;
	}
	~UserInfo();
	void GetLocktime(int& locktime, int& timeout, int& settime)
	{
		time_t now = Timer::GetTime();
		if((settime = autolock.GetValue(LOCKSET_TIME_GM)) > 0)
		{
			int gmlocktime = autolock.GetValue(LOCKTIME_GM);
			if (gmlocktime < 0 || now - settime <= gmlocktime)
			{
				locktime = -1;	//-1代表永久安全琐
				timeout = 0;
				return;
			}
		}

		settime = autolock.GetValue(LOCKSET_TIME);
		locktime = autolock.GetValue(LOCKTIME_NOW);
		if(settime!=-1)
		{
			int timenew = autolock.GetValue(LOCKTIME_NEW);
			if(timenew>locktime || now-settime>86400*3)
				locktime = timenew;
		}
		if(locktime<=0)
			locktime = 60;

		timeout = locktime - now + logintime;
		if(timeout<0)
			timeout = 0;
	}
	int SetLocktime(int locktime)
	{
		time_t now = Timer::GetTime();
		int gmsettime;
		if((gmsettime = autolock.GetValue(LOCKSET_TIME_GM)) > 0)
		{
			int gmlocktime = autolock.GetValue(LOCKTIME_GM);
			if (gmlocktime < 0 || now - gmsettime <= gmlocktime)
			{
				return -1;	//存在未过期的GM安全琐则不允许玩家进行设置
			}
		}
		int settime = autolock.GetValue(LOCKSET_TIME);
		int timenow = 0;
		if(settime>0)
		{
			if(now-settime<60)
				return -1;
			int timenew = autolock.GetValue(LOCKTIME_NEW);
			timenow = autolock.GetValue(LOCKTIME_NOW);
			if(now-settime>86400*3 || timenew>timenow)
			{
				timenow = timenew;
				autolock.SetValue(LOCKTIME_NOW, timenew);
			}
		}
		autolock.SetValue(LOCKTIME_NEW, locktime);
		autolock.SetValue(LOCKSET_TIME, now);
		Log::formatlog("autolock","set:userid=%d:roleid=%d:newtime=%d:oldtime=%d", userid, roleid, locktime, timenow);
		return 0;
	}
	int GMSetLocktime(int locktime, bool force = false)
	{
		time_t now = Timer::GetTime();
		int settime = autolock.GetValue(LOCKSET_TIME_GM);
		if(!force && settime > 0 && now - settime < 60)
			return -1;
		autolock.SetValue(LOCKSET_TIME_GM, locktime==0 ? -1 : now);
		autolock.SetValue(LOCKTIME_GM, locktime);
		return 0;
	}
	bool CheckRoleLogin(int rid)
	{
		switch(role_status[rid % MAX_ROLE_COUNT])
		{
			case _ROLE_STATUS_NORMAL:
				return true;
		}
		return false;
	}
	bool CheckDeleteRole(int rid)
	{
		switch(role_status[rid % MAX_ROLE_COUNT])
		{
			case _ROLE_STATUS_NORMAL: 
				return true;
		}
		return false;
	}
	bool CheckUndoDeleteRole(int rid)
	{
		switch(role_status[rid % MAX_ROLE_COUNT])
		{
			case _ROLE_STATUS_READYDEL:
				return true;
		}
		return false;
	}
	bool CheckSellRole(int rid)
	{
		switch(role_status[rid % MAX_ROLE_COUNT])
		{
			case _ROLE_STATUS_NORMAL:
				return true;
		}
		return false;
	}
	bool CheckCancelSellRole(int rid)
	{
		switch(role_status[rid % MAX_ROLE_COUNT])
		{
			case _ROLE_STATUS_FROZEN:
				return true;
		}
		return false;
	}	

	//cross server related
	void UpdateCrossInfoData(int roleid, CrossInfoData& data) 
	{
		cross_info[roleid % MAX_ROLE_COUNT] = data;
	}
	
	CrossInfoData* GetCrossInfo(int roleid)
	{
		if(!rolelist.IsRoleExist(roleid)) return NULL;
		return &cross_info[roleid % MAX_ROLE_COUNT];
	}
	
	bool IsRoleCrossLocked(int roleid)
	{
		switch(role_status[roleid % MAX_ROLE_COUNT])
		{
			case _ROLE_STATUS_CROSS_LOCKED:
				return true;
		}
		
		return false;
	}
	//cross server end
};

class PlayerInfo
{
public:
	int roleid;
	int userid;
	UserInfo* user;
	Octets    name;
	bool ingame;
	int  gameid;
	unsigned int linksid;
	unsigned int localsid;
	int cls;
	unsigned char emotion;
	unsigned int  spouse;
	int level;
	int reincarnation_times;
	
	// friend system relative data
	int friend_ver; // -1:unavailable, 0:untouched, >0: modified
	GGroupInfoVector  groups;
	GFriendInfoVector friends;
	GFriendExtInfoVector friendextinfo;
	GSendAUMailRecordVector sendaumailinfo;
	
	PlayerInfo(UserInfo* u, int rid) : roleid(rid), user(u), ingame(0)
	{
		userid = u->userid;
		gameid = u->gameid;
		linksid = u->linksid;
		localsid = u->localsid;
		friend_ver = -1;
		cls = 0;
		emotion = 0;
		spouse = 0;
		level = 0;
		reincarnation_times = 0;
	}
	PlayerInfo() { }
	~PlayerInfo();

	bool IsGM() {  return (user->gmstatus&GMSTATE_ACTIVE)!=0; }
};

#define MAX_PLAYER_NUM_DEFAULT 5000

class UserContainer
{
	typedef std::map<int,UserInfo>   UserMap;
	typedef std::map<int,PlayerInfo*> RoleMap;
	typedef std::set<int> LanIpSet;
	UserMap usermap;
	RoleMap rolemap;
	LanIpSet lan_ip_set; //用来记录内网ip的列表，ip属于该列表内的玩家，不受登陆总人数的限制
	Thread::RWLock	locker;

	//rolename hash map 
	typedef __gnu_cxx::hash_map<Octets,int> RolenameMap;
	RolenameMap rolenamemap;
	Thread::Mutex	locker_rolename;

	size_t		max_player_num;
	size_t		fake_max_player_num;
	Thread::Mutex locker_maxplayer;

	//cross server related
	std::set<int>	remoteonlineset;//目前只做统计用
	
	static UserContainer	instance;
public:
	UserContainer() : max_player_num(MAX_PLAYER_NUM_DEFAULT), fake_max_player_num(MAX_PLAYER_NUM_DEFAULT)
	{ }
	~UserContainer() { }
	static UserContainer & GetInstance() { return instance; }
	size_t Size() { Thread::RWLock::RDScoped l(locker);	return usermap.size();	}
	size_t SizeNoLock() { return usermap.size();	}

	Thread::RWLock & GetLocker() { return locker; }

	bool FindRoleId( const Octets& name, int& roleid )
	{
		Thread::Mutex::Scoped l(locker_rolename);
		RolenameMap::iterator it = rolenamemap.find(name);
		if(it!=rolenamemap.end())
		{
			roleid = (*it).second;
			return true;
		}
		return false;
	}

	void InsertName( const Octets& name, int roleid )
	{
		Thread::Mutex::Scoped l(locker_rolename);
		rolenamemap[name] = roleid;
	}

	void EraseName( const Octets& name )
	{
		Thread::Mutex::Scoped l(locker_rolename);
		RolenameMap::iterator it=rolenamemap.find(name);
		if ( it!=rolenamemap.end() )
		{
			rolenamemap.erase( it );
		}
	}

	bool FindUser( int userid, UserInfo& info )
	{
		Thread::RWLock::RDScoped l(locker);
		UserMap::iterator it = usermap.find(userid);
		if(it!=usermap.end())
		{
			info = (*it).second;
			return true;
		}
		return false;
	}
	void InsertUser( int userid, UserInfo& info )
	{
		Thread::RWLock::WRScoped l(locker);
		usermap[userid] = info;
	}

	void EraseUser( int userid )
	{
		Thread::RWLock::WRScoped l(locker);
		usermap.erase( userid );
	}

	void EraseUserNoLock( int userid )
	{
		usermap.erase( userid );
	}

	class IQueryUser
	{   
	public:
		virtual ~IQueryUser() { } 
		virtual bool Update( int userid, UserInfo & info ) = 0;
	};
	void WalkUser( IQueryUser& q )
	{
		Thread::RWLock::RDScoped l(locker);
		for( UserMap::iterator it = usermap.begin(), ite = usermap.end(); it != ite; ++it )
		{
			q.Update( (*it).first, (*it).second );
		}
	}
	void DeleteWalkUser( IQueryUser& q )
	{
		Thread::RWLock::RDScoped l(locker);
		{
			MapEraser<UserMap> e(usermap);
			for( UserMap::iterator it = usermap.begin(), ite = usermap.end(); it != ite; ++it )
			{
				if( !q.Update( (*it).first, (*it).second ) )
					e.push( (*it).first );
			}
		}
	}
	void PartlyWalkUser( int& beginid, int count, IQueryUser& q )
	{
		Thread::RWLock::RDScoped l(locker);
		UserMap::iterator it;
		UserMap::iterator ite = usermap.end();
		if( -1 == beginid )
			it = usermap.begin();
		else
			it = usermap.lower_bound(beginid);
		int n = 0;
		for( ; it != ite && n < count; ++n, ++it )
		{
			q.Update( (*it).first, (*it).second );
		}
		beginid = ( it != usermap.end() ? (*it).first : -1 );
	}

	UserInfo* FindUser( int userid )
	{
		UserMap::iterator it = usermap.find(userid);
		if(it!=usermap.end())
			return &((*it).second);
		return NULL;
	}

        PlayerInfo* FindRole( int roleid )
        {
                RoleMap::iterator it = rolemap.find(roleid);
                if(it!=rolemap.end())
                        return it->second;
                return NULL;
        }

	PlayerInfo* FindRoleOnline( int roleid )
	{
		PlayerInfo* pinfo = NULL;
		RoleMap::iterator it = rolemap.find(roleid);
		if(it!=rolemap.end())
			pinfo = it->second;
		return (pinfo && pinfo->ingame) ? pinfo : NULL;
	}

	UserInfo* FindUser(int userid,unsigned int link_sid,unsigned int localsid);
	
	std::string GetUserIP( int userid );

	void SetPlayerLimit(size_t num,size_t fake_num)
	{
		Thread::Mutex::Scoped l(locker_maxplayer);	
		max_player_num = num; 
		fake_max_player_num = fake_num;
	}

	size_t GetPlayerLimit() 
	{ 
		Thread::Mutex::Scoped l(locker_maxplayer);
		return max_player_num;
	}

	size_t GetFakePlayerLimit()
	{
		Thread::Mutex::Scoped l(locker_maxplayer);	
		return fake_max_player_num;
	}

	void UserLogin(int userid, const Octets & account, int linksid, int localsid, bool isgm, int type, int data, int ip, const Octets& iseckey, const Octets& oseckey, bool notify_client = true);
	//void UserLogout(UserInfo * pinfo, bool iskick=false);
	void UserLogout(UserInfo * pinfo, char kick_type = 0, bool force = false);
	void RoleLogout(UserInfo * pinfo, bool is_cross_related = false/*cross server related*/);
	void ContinueLogin(int userid, bool result);
	bool OnPlayerLogout(PlayerLogout& cmd);
	PlayerInfo* RoleLogin( UserInfo* user, int roleid)
	{
		if(user->role)
		{
			rolemap.erase(user->role->roleid);
			delete user->role;
		}
		PlayerInfo* role = new PlayerInfo(user, roleid);
		user->role = role;
		user->roleid = roleid;
		user->status = _STATUS_READYGAME;
		rolemap[roleid] = role; 
		return role;
	}

	bool CheckSwitch(PlayerInfo* info, int roleid, int linkid,unsigned int localsid, Protocol::Manager *manager,unsigned int sid);
	
	/**
	 * 初始化内网ip列表
	 * @param list_str ip列表的字符串，各个ip间目前用逗号分隔
	 */
	void InitLanIpList(const std::string list_str)
	{
		lan_ip_set.clear();
		if( list_str.length() <= 0 ) return;

		char* delim = ",";
		char* buffer = new char[list_str.length() + 1];
		if( NULL == buffer ) return;
		
		memcpy( buffer, list_str.c_str(), list_str.length() );
		buffer[list_str.length()] = 0;

		char* token = strtok( buffer, delim );
		while( NULL != token ) { 
			struct in_addr addr;
			inet_pton(AF_INET, token, (void*)&addr);
			lan_ip_set.insert( addr.s_addr );
			token = strtok( NULL, delim );
		}       

		delete [] buffer; 
	}

	/**
	 * 判断是否内网ip
	 * @return true:内网ip false:外部ip
	 */
	bool IsLanIp(int ip)
	{
		return (lan_ip_set.find(ip) != lan_ip_set.end());
	}

	//cross server related
	int ClearRemoteUsers();
	int DisconnectZoneUsers(int zoneid);

	void InsertRemoteOnline(int userid)
	{
		remoteonlineset.insert(userid);
	}
	
	void EraseRemoteOnline(int userid)
	{
		remoteonlineset.erase(userid);
	}

	size_t RemoteOnlineSize()
	{
		return remoteonlineset.size();
	}
	
	CrossInfoData* GetRoleCrossInfo(int roleid);
};

};

#endif

