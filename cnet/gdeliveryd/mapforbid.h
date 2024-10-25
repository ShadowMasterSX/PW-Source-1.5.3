#ifndef __GNET_MAPFORBID_H
#define __GNET_MAPFORBID_H

#include <map>

#include "thread.h"
#include "groleforbid"
#include "forbid.hxx"

#define MAX_REMOVETIME 120
namespace GNET
{

class ForbidMap
{
protected:	
	typedef std::map<int/*userid*/,GRoleForbid> Map;
	Map map;
	Thread::Mutex locker;
public:
	ForbidMap(const char* lock_name) : locker(lock_name) { }
	virtual ~ForbidMap() { }
	
	size_t Size() { Thread::Mutex::Scoped l(locker);	return map.size();	}
	void Update( int update_time )
	{
		Thread::Mutex::Scoped l(locker);
		{
			MapEraser<Map> e(map);
			for (Map::iterator it = map.begin(), ite = map.end(); it != ite; ++it )
			{
				(*it).second.time -= update_time;
				if ( (*it).second.time <= 0 )
					e.push( (*it).first );
			}
		}
	}
	bool GetForbid( int userid, GRoleForbid& forbid )
	{
		Thread::Mutex::Scoped l(locker);
		Map::iterator it = map.find( userid );
		if (it != map.end())
		{
			forbid = (*it).second;
			return true;
		}
		return false;
	}
	void SetForbid( int userid, GRoleForbid& forbid )
	{
		Thread::Mutex::Scoped l(locker);
		map[userid] = forbid;
	}
	void RmvForbid( int userid )
	{
		Thread::Mutex::Scoped l(locker);
		map.erase(userid);
	}
};
class ForbidLogin : public ForbidMap
{
	static ForbidLogin	instance;
	//forbid login flag,if this flag is true, all login is forbiddened
	bool blAllowLoginGlobal;
public:
	ForbidLogin() : ForbidMap("ForbidLogin::locker"), blAllowLoginGlobal(true) { }
	static ForbidLogin & GetInstance() { return instance; }

	bool GetForbidLogin( int userid, GRoleForbid& forbid ) {
		return GetForbid(userid,forbid);
	}

	void SetForbidLogin( int userid, GRoleForbid& forbid ) {
		SetForbid(userid,forbid);
	}
	void SetForbidLoginIfLonger( int userid, GRoleForbid& forbid ) {
		//ForbidLogin map 中类型都为FBD_FORBID_LOGIN，为了与客户端兼容
		forbid.type = Forbid::FBD_FORBID_LOGIN;
		GRoleForbid old;
		if (GetForbidLogin(userid, old))
		{
			//这里的time都是封禁剩余时间，每60秒更新一次
			if (forbid.time > old.time)
				SetForbid(userid, forbid);
		}
		else
			SetForbid(userid, forbid);
	}
			
	void RmvForbidLogin( int userid ) {
		RmvForbid(userid);
	}
	
	void ForbidLoginGlobal()
	{
		Thread::Mutex::Scoped l(locker);
		blAllowLoginGlobal = false;
	}
	void AllowLoginGlobal()
	{
		Thread::Mutex::Scoped l(locker);
		blAllowLoginGlobal = true;
	}
	bool IsLoginAllowedGlobal()
	{
		Thread::Mutex::Scoped l(locker);
		return blAllowLoginGlobal;
	}
};

class ForbidRoleLogin : public ForbidMap
{
	static ForbidRoleLogin	instance;
public:
	ForbidRoleLogin() : ForbidMap("ForbidRoleLogin::locker") { }
	static ForbidRoleLogin & GetInstance() { return instance; }

	bool GetForbidRoleLogin( int roleid, GRoleForbid& forbid ) {
		return GetForbid(roleid,forbid);
	}

	void SetForbidRoleLogin( int roleid, GRoleForbid& forbid ) {
		SetForbid(roleid,forbid);
	}
	void RmvForbidRoleLogin( int roleid ) {
		RmvForbid(roleid);
	}
	
};

class ForbidTrade : public ForbidMap
{
	static ForbidTrade	instance;
public:
	ForbidTrade() : ForbidMap("ForbidTrade::locker") { }
	~ForbidTrade() { }
	static ForbidTrade & GetInstance() { return instance; }

	bool GetForbidTrade( int roleid, GRoleForbid& forbid ) {
		return GetForbid(roleid,forbid);
	}

	void SetForbidTrade( int roleid, GRoleForbid& forbid ) {
		SetForbid(roleid,forbid);
	}

};

class ForbidSellPoint : public ForbidMap
{
	static ForbidSellPoint instance;
public:
	ForbidSellPoint() : ForbidMap("ForbidSellPoint::locker") { }
	~ForbidSellPoint() { }
	static ForbidSellPoint & GetInstance() { return instance; }
};

class ForbidUserTalk: public ForbidMap //主要用于向跨服同步账号禁言信息
{
	static ForbidUserTalk instance;
public:
	ForbidUserTalk() : ForbidMap("ForbidUserTalk::locker") { }
	~ForbidUserTalk() { }
	static ForbidUserTalk & GetInstance() { return instance; }

	bool GetForbidUserTalk( int userid, GRoleForbid& forbid ) {
		return GetForbid(userid,forbid);
	}

	void SetForbidUserTalk( int userid, GRoleForbid& forbid ) {
		SetForbid(userid,forbid);
	}
};

class ForbiddenUsers
{
	//typedef std::set<int> Set;
	//Set set;
	friend class RemoteLoggingUsers;

public:	
	struct userinfo_t
	{
		int roleid;
		int status;
		int addtime;
		userinfo_t() : roleid(-1),status(0) {  addtime=time(NULL); }
		userinfo_t( int r,int s ) : roleid(r),status(s) { addtime=time(NULL); }
	};
	typedef std::map<int,userinfo_t> Set; //map userid to userinfo
	typedef std::vector<userinfo_t>  UserList;
private:	
	Set set;
	Thread::Mutex locker;
	static ForbiddenUsers	instance;
public:
	ForbiddenUsers() : locker("ForbiddenUsers::locker"){}
	~ForbiddenUsers() {}
	static ForbiddenUsers & GetInstance() { return instance; }
	size_t Size() { Thread::Mutex::Scoped l(locker);	return set.size();	}

	bool IsExist(int userid)
	{
		Thread::Mutex::Scoped 	l(locker);
		return set.find(userid)!=set.end();
	}	
	void Push(int userid,int roleid,int status )
	{
		Thread::Mutex::Scoped 	l(locker);
		set[userid]=userinfo_t(roleid,status);
	}
	size_t Pop(int userid)
	{
		Thread::Mutex::Scoped 	l(locker);
		return set.erase(userid);
	}
	size_t size()
	{
		Thread::Mutex::Scoped 	l(locker);
		return set.size();
	}
	void CheckTimeoutUser();
};

class RemoteLoggingUsers
{
	//临时禁止登陆 同步使用
	enum {
		DEFAULT_REMOTE_LOGGING_TIMEOUT = 120,
	};
	
	ForbiddenUsers user_map;
	int logging_timeout;
	RemoteLoggingUsers();
	
public:
	~RemoteLoggingUsers() { }
	static RemoteLoggingUsers& GetInstance() { static RemoteLoggingUsers instance; return instance; }
	bool IsExist(int userid) { return user_map.IsExist(userid); }
	void Push(int userid, int roleid, int status) { user_map.Push(userid, roleid, status); }
	void Pop(int userid) { user_map.Pop(userid); }
	void CheckTimeoutUser();
};

class RemoveForbidden : public Thread::Runnable
{
public:
	RemoveForbidden(int priority=1):
		Runnable(priority)
   	{}
	~RemoveForbidden() {}
	void Run();
};

/* Check forbidlogin user map periodically, and remove timeout items
 */  
class CheckTimer : public Thread::Runnable
{
	int update_time;
	CheckTimer(int _time,int _proir=1) : Runnable(_proir),update_time(_time) { }
public:
	static CheckTimer* GetInstance(int _time=60,int _proir=1) {
		static CheckTimer instance(_time,_proir);
		return &instance;
	}	

	void Run()
	{
		ForbidLogin::GetInstance().Update( update_time );
		ForbidRoleLogin::GetInstance().Update( update_time );
		ForbidTrade::GetInstance().Update( update_time );
		ForbidSellPoint::GetInstance().Update( update_time );
		ForbidUserTalk::GetInstance().Update( update_time );

		Thread::HouseKeeper::AddTimerTask(this,update_time);
	}
};

};

#endif

