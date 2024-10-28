#ifndef __GNET_CROSS_SYSTEM_H
#define __GNET_CROSS_SYSTEM_H

#include "mapuser.h"

namespace GNET
{

class UserIdentityCache: public Timer::Observer
{
public:
	class Identity
	{
	public:
		int roleid;
		int remote_roleid;
		int src_zoneid;
		int ip;
		Octets iseckey;
		Octets oseckey;
		Octets account;
		Octets rand_key;
		int logintime; //切换服务器之前的账号登陆时间 用于安全锁继续计时
		int addtime;
		char au_isgm;
		int au_func;
		int au_funcparm;
		ByteVector auth;
		char usbbind;
		int reward_mask;
		GRoleForbid forbid_talk;

		Identity(int _roleid = 0, int _remote_roleid = 0, int _zoneid = 0, int _ip = 0, const Octets & _ikey = Octets(), const Octets & _okey=Octets(), const Octets & _account=Octets(), 
			const Octets & _rand=Octets(), int _login = 0, char _isgm = 0, int _func = 0, int _funcparm = 0, const ByteVector& _auth = ByteVector(), char _usbbind = 0, 
			int _reward_mask = 0, const GRoleForbid & _forbid = GRoleForbid()):
			roleid(_roleid), remote_roleid(_remote_roleid), src_zoneid(_zoneid), ip(_ip), iseckey(_ikey), oseckey(_okey), account(_account), 
			rand_key(_rand), logintime(_login), au_isgm(_isgm), au_func(_func), au_funcparm(_funcparm), auth(_auth), usbbind(_usbbind), reward_mask(_reward_mask),
			forbid_talk(_forbid)
		{ 
			addtime=time(NULL); 
		}
	};
	
	typedef std::map<int/*userid*/, Identity> IdentityMap;
	
private:
	enum {
		DEFAULT_CACHE_MAXTIME = 90,
	};
	
	IdentityMap identity_map;
	int cache_max_time;
	UserIdentityCache();
	
	//static UserIdentityCache instance;
	
public:
	static UserIdentityCache* GetInstance() { static UserIdentityCache instance;  return &instance; }

	bool Exist(int userid)
	{
		IdentityMap::const_iterator it = identity_map.find(userid);
		return (it != identity_map.end());
	}
	
	bool Find(int userid, Identity& iden)
	{
		IdentityMap::const_iterator it = identity_map.find(userid);
		if(it == identity_map.end()) return false;
		
		iden = Identity(it->second);
		return true;
	}
	
	void Insert(int userid, const Identity& iden)
	{
		identity_map[userid] = iden;
		LOG_TRACE("UserIdentityCache insert user %d addtime %d", userid, identity_map[userid].addtime);
	}
	
	void Remove(int userid)
	{
		identity_map.erase(userid);
		LOG_TRACE("UserIdentityCache remove user %d", userid);
	}
	
	bool UserIdentityCache::UpdateRoleCrsInfo(int userid, int new_roleid)
	{
		IdentityMap::iterator it = identity_map.find(userid);
		if(it == identity_map.end()) return false;
		
		Identity& iden = it->second;
		iden.roleid = new_roleid;
		return true;
	}

	void Update()
	{
		//LOG_TRACE("UserIdentityCache update cache size %d", identity_map.size());
		int now = time(NULL);
		
		for(IdentityMap::iterator it = identity_map.begin(); it != identity_map.end();) {
			if (now - it->second.addtime > cache_max_time) {
				LOG_TRACE("UserIdentityCache erase user %d role %d, remote_roleid %d", it->first, it->second.roleid, it->second.remote_roleid);
				STAT_MIN5("PlayerIdentityTimeout", 1);
				identity_map.erase(it++);
			} else {
				++it;
			}
		}
	}
};

class DelayRolelistTask : public Thread::Runnable
{
private:
	int userid;
	int roleid;
	static std::map<int, RoleInfo> roleinfo_map;
	
public:
	DelayRolelistTask(int uid, int rid) : Runnable(1), userid(uid), roleid(rid){ }
	
	static void InsertRoleInfo(const RoleInfo& info)
	{
		roleinfo_map[info.roleid] = info;
	}
	
	static void RemoveRoleInfo(int roleid)
	{
		roleinfo_map.erase(roleid);
	}
	
	static void OnRecvInfo(int uid, int rid);
	
	void Run()
	{
		OnRecvInfo(userid, roleid);
		delete this;
	};
};

enum
{
	UNCK_BEG = CARNIVAL_DOOR_UNKEY_BEG + CT_TYPE_BEG,
	UNCK_END = CARNIVAL_DOOR_UNKEY_BEG + CT_TYPE_END,
};

// 普通服跳中央服的限制
class CrossGuardServer : public Timer::Observer 
{
	struct data_node
	{
		typedef std::vector<int> ZONE_LIST;
		
		int day;
		int beg;
		int end;
		ZONE_LIST zonelist;
		
		data_node() : day(-1),beg(-1),end(-1) {}
		data_node(int d,int b,int e,int* zlist, int zcount) : day(d),beg(b),end(e)
		{
			for(int n = 0; n < zcount; n++)
			{
				zonelist.push_back(zlist[n]);
			}
		}
		bool check(int wday, int second_of_day)
		{
			return (wday == day) && (second_of_day >= beg) && (second_of_day <= end);
		}
	};
	typedef std::vector<data_node> DateList;
	typedef std::map<int,DateList> DateMap;

	DateMap _open_date_map;
	bool _init;
	int  _tick;
	int  _debug_day;
	int  _debug_second;
public:
	CrossGuardServer() : _init(false),_tick(-1),_debug_day(-1),_debug_second(-1) {}

	void SetAdjustTime(int day,int second) { _debug_day = day; _debug_second = second;}
	void Initialize();
	void Update();
	void Register(CARNIVAL_TYPE key, int day, int begtime, int endtime, int* zlist, int zcount);
public:	
	static CrossGuardServer* GetInstance()
	{
		static CrossGuardServer ins;
		return &ins;
	}
};

class CrossGuardClient
{
	bool _init;
	bool _debug_pass;
public:
	CrossGuardClient() : _init(false),_debug_pass(false) {}
	void Initialize();
	bool CanCross();
	void OnUpdate(IntVector& clist);
	void OnPlayerCross(int roleid,short type,int64_t mnfid,bool backflag);
	void SetDebug(bool b) {_debug_pass = b;}
public:	
	static CrossGuardClient* GetInstance()
	{
		static CrossGuardClient ins;
		return &ins;
	}
};

class CrossChat;
class CrossChat_Re;

class CrossChatServer
{
	bool _init;
public:	
	CrossChatServer() : _init(false) {}
	void OnSend(int roleid,unsigned char channel,unsigned char emotion,const Octets& name,const Octets& msg,const Octets& data,int zoneid = -1);
	void OnRecv(const CrossChat& msg);
	void Initialize();

public:
	static CrossChatServer* GetInstance()
	{
		static CrossChatServer ins;
		return &ins;
	}
};

class CrossChatClient : public IntervalTimer::Observer
{
	typedef std::map<int,CrossChat*> MSG_CACHE_MAP;

	int  _sn;
	bool _init;
	MSG_CACHE_MAP _cache_map;
public:	
	CrossChatClient() : _sn(0),_init(false) {}
	void OnSend(int roleid,unsigned char channel,unsigned char emotion,const Octets& name,const Octets& msg,const Octets& data);
	void OnRecv(const CrossChat_Re& msg);
	void Initialize();
	bool Update();

public:
	static CrossChatClient* GetInstance()
	{
		static CrossChatClient ins;
		return &ins;
	}
};

class CrossSoloChallengeRank;
class CrossSoloChallengeRank_Re;

class CrossSoloRankServer
{
	bool _init;
public:	
	CrossSoloRankServer() : _init(false) {} 
	void Initialize();
	void OnRecv(const CrossSoloChallengeRank& msg);
	void OnRecv(const CrossSoloChallengeRank_Re& msg);
public:
	static CrossSoloRankServer* GetInstance()
	{
		static CrossSoloRankServer ins;
		return &ins;
	}
};

class CrossSoloRankClient : public IntervalTimer::Observer
{
	enum CSR_STATE
	{
		CSR_UNINIT,
		CSR_NORMAL,
		CSR_SYNC_SEND,
		CSR_SYNC_RECV,
	};

	bool _init;
	int  _state;
public:	
	CrossSoloRankClient() : _init(false),_state(CSR_UNINIT) {} 
	void Initialize();
	void OnRecv(const CrossSoloChallengeRank& msg);
	void OnRecv(const CrossSoloChallengeRank_Re& msg);
	bool Update();

    int SetState(int state)
    {
        int old_state = _state;
        if ((state >= CSR_UNINIT) && (state <= CSR_SYNC_RECV)) _state = state;
        return old_state;
    }

public:
	static CrossSoloRankClient* GetInstance()
	{
		static CrossSoloRankClient ins;
		return &ins;
	}
};
}

#endif
