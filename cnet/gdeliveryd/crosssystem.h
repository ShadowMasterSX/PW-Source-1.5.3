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

}

#endif
