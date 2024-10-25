#ifndef __GNET_MAPPASSWD_H
#define __GNET_MAPPASSWD_H

#include <map>
#include <ext/hash_map>
#include <set>

#include "hashstring.h"
#include "thread.h"

#include <itimer.h>
#include <vector>

#include "mappasswordvalue"
#include "mappassworddata"

namespace GNET
{
class MPassword
{
	int    userid;
	int    algorithm;
	Octets password;
	Octets matrix;
	Octets seed;
	Octets pin;
	int	   rtime;
    int    refreshtime;
	public:
	MPassword() { }
	MPassword(int u, int algo, const Octets& p, const Octets& m, const Octets& _seed, const Octets& _pin, int _rtime, int _refreshtime) : userid(u),algorithm(algo),password(p),matrix(m),seed(_seed),pin(_pin),rtime(_rtime), refreshtime(_refreshtime)  {}
	MPassword(const MPassword& r) : userid(r.userid),algorithm(r.algorithm),password(r.password),matrix(r.matrix),seed(r.seed),pin(r.pin),rtime(r.rtime), refreshtime(r.refreshtime)  {}
    MPassword(const MapPasswordValue& r) : userid(r.userid), algorithm(r.algorithm), password(r.password), matrix(r.matrix), seed(r.seed), pin(r.pin), rtime(r.rtime), refreshtime(r.refreshtime) {}

    void GetPassword(int& uid, int& algo, Octets& p, Octets& m, Octets& _seed, Octets& _pin, int& _rtime, int& _refreshtime)
	{
		uid  = userid;
		algo = algorithm;
		p = password;
		m = matrix;
		_seed = seed;
		_pin = pin;
		_rtime = rtime;
        _refreshtime = refreshtime;
	}

    int GetRefreshTime() const
    {
        return refreshtime;
    }

    operator MapPasswordValue() const
    {
        return MapPasswordValue(userid, algorithm, password, matrix, seed, pin, rtime, refreshtime);
    }
};

class Passwd : public IntervalTimer::Observer
{
	typedef __gnu_cxx::hash_map<Octets/*username*/,MPassword> Map;
	Map map;
	//完美神盾帐号
	std::set<int> usbuserset;
	std::map<int,int> userrewardmap;		//用来缓存账号信息补填的奖励
	Thread::Mutex locker;
	static Passwd	instance;

    bool initialized;
    int updatetime;
public:
	Passwd() : locker("Passwd::locker"), initialized(false), updatetime(0) { }
	virtual ~Passwd() { }
	static Passwd & GetInstance() { return instance; }
	size_t Size() { Thread::Mutex::Scoped l(locker);	return map.size();	}

	bool GetPasswd( Octets username, int& userid, Octets& passwd )
	{
		Thread::Mutex::Scoped l(locker);
		Map::iterator it = map.find( username );
		if (it != map.end())
		{
			Octets matrix;
			int algorithm;
			Octets seed;
			Octets pin;
			int rtime;
            int refreshtime;
			it->second.GetPassword(userid, algorithm, passwd, matrix, seed, pin, rtime, refreshtime);
			return true;
		}
		return false;
	}
	
	bool GetPasswd( Octets username, int& userid, Octets& passwd, Octets& matrix, int& algorithm )
	{
		Thread::Mutex::Scoped l(locker);
		Map::iterator it = map.find( username );
		if (it != map.end())
		{
			Octets seed;
			Octets pin;
			int rtime;
            int refreshtime;
			it->second.GetPassword(userid, algorithm, passwd, matrix, seed, pin, rtime, refreshtime);
			return true;
		}
		return false;
	}
	
	bool GetPasswd( Octets username, int& userid, Octets& passwd, Octets& matrix, int& algorithm, Octets& seed, Octets& pin, int& rtime)
	{
		Thread::Mutex::Scoped l(locker);
		Map::iterator it = map.find( username );
		if (it != map.end())
		{
            int refreshtime;
			it->second.GetPassword(userid, algorithm, passwd, matrix, seed, pin, rtime, refreshtime);
			return true;
		}
		return false;
	}
	
	void SetPasswd( Octets username, int userid, Octets& passwd )
	{
		Thread::Mutex::Scoped l(locker);
		map[username] = MPassword(userid, 0, passwd, Octets(), Octets(), Octets(), 0, time(NULL));
	}

	void SetPasswd( Octets username, int userid, Octets& passwd, Octets& matrix, int algorithm )
	{
		Thread::Mutex::Scoped l(locker);
		map[username] = MPassword(userid, algorithm, passwd, matrix, Octets(), Octets(), 0, time(NULL));
	}
	
	void SetPasswd( Octets username, int userid, Octets& passwd, Octets& matrix, int algorithm, Octets& seed, Octets& pin, int rtime)
	{
		Thread::Mutex::Scoped l(locker);
		map[username] = MPassword(userid, algorithm, passwd, matrix, seed, pin, rtime, time(NULL));
	}

	void ClearPasswd(Octets username)
	{
		Thread::Mutex::Scoped l(locker);
		map.erase(username);
	}
	
	void SetUsbUser(int userid)
	{
		Thread::Mutex::Scoped l(locker);
		usbuserset.insert(userid);
	}
	void ClearUsbUser(int userid)
	{
		Thread::Mutex::Scoped l(locker);
		usbuserset.erase(userid);
	}
	bool IsUsbUser(int userid)
	{
		Thread::Mutex::Scoped l(locker);
		return (usbuserset.find(userid) != usbuserset.end());
	}
	void InsertUserReward(int userid, int rewardmask)
	{
		Thread::Mutex::Scoped l(locker);
		userrewardmap[userid] = rewardmask;
	}
	int GetUserReward(int userid)
	{
		Thread::Mutex::Scoped l(locker);
		std::map<int,int>::iterator it = userrewardmap.find(userid);
		return it == userrewardmap.end() ? 0 : it->second;
	}

    bool Update();
    bool Initialize();

    void OnDBConnect(Protocol::Manager* manager, int sid);
    void OnDBLoad(const std::vector<MapPasswordData>& data, bool finish);

private:
    bool GetMapPasswordDataNoLock(std::vector<MapPasswordData>& data, Octets& handle);

};

}

#endif

