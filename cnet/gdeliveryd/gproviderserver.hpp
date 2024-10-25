#ifndef __GNET_GPROVIDERSERVER_HPP
#define __GNET_GPROVIDERSERVER_HPP

#include "protocol.h"
#include "macros.h"


namespace GNET
{

class GMControlGame;
class GMControlGame_Re;

class GProviderServer : public Protocol::Manager, public Timer::Observer
{
public:	
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
		bool IsEnclosed(point_t pt) { return (pt.x>=left && pt.x<=right) && (pt.z>=bottom && pt.z<=top ); }
	};
	struct gameserver_t
	{
		unsigned int sid;
		region_t	region;
		int			worldtag;
		gameserver_t() { }
		gameserver_t(unsigned int _sid,const region_t& _r,int _wt) : sid(_sid), region(_r), worldtag(_wt) { }
	};
private:
	static GProviderServer instance;
	size_t		accumulate_limit;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	
	int m_provider_server_id;

	void Update()
	{
		OnGameControlResCheck();
	}

	// for worldtag broadcast
private:	
	enum { DEFAULT_GAMECONTROL_TIMEOUT = 15 };
	enum { GAMECONTROL_TIMEOUT = -2, GAMECONTROL_SENDFAIL = -1
			, GAMECONTROL_SUCCESS = 0, GAMECONTROL_FAIL = 1 };
	enum { GAMECONTROL_INVALID_WORLDTAG = -1 };
	struct ControlGameRes
	{
		int tick;
		unsigned int client_sid;
		GMControlGame_Re* re;
	};
	typedef std::map<int, ControlGameRes> ControlGameResMap;
	ControlGameResMap controlgameres_map;
	int send_xid;

	struct phone_gs_t
	{
		unsigned int num;
		bool isconnect;
	};
	typedef std::map<int, phone_gs_t > PhoneGSPlayerNumMap;
	PhoneGSPlayerNumMap phonegsplayernum_map;

	void OnGameControlResCheck();
public:
	void DeliverGameControl(GMControlGame *gmcg, unsigned int sid);
	void DeliverGameControlRes(const GMControlGame_Re *gmcgr, unsigned int sid);

public:
	static GProviderServer *GetInstance() { return &instance; }
	std::string Identification() const { return "GProviderServer"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GProviderServer() : 
		accumulate_limit(0),
		m_provider_server_id(_PROVIDER_ID_INVALID),
		send_xid(0),
		locker_gameservermap("GProviderServer::locker_gameservermap"),
		locker_phonegsplayernummap("GProviderServer::locker_phonegsplayernummap")
   	{ Timer::Attach(this); }
	
	Thread::RWLock locker_gameservermap;
	Thread::RWLock locker_phonegsplayernummap;
	//typedef std::map<int,Session::ID> GameServerMap;
	typedef std::map<int,gameserver_t> GameServerMap;
	GameServerMap gameservermap;
	void SetProviderServerID(int ps_id) { m_provider_server_id=ps_id; } 
	int  GetProviderServerID() { return m_provider_server_id; } 

	bool DispatchProtocol(int game_id,const Protocol *protocol)
	{
		Thread::RWLock::RDScoped l(locker_gameservermap);
		GameServerMap::const_iterator it=gameservermap.find(game_id);
		if (it==gameservermap.end()) return false;
		return this->Send((*it).second.sid,protocol);
	}
	bool DispatchProtocol(int game_id,const Protocol &protocol)
	{
		return DispatchProtocol(game_id,&protocol);
	}
	bool DispatchProtocol(int game_id,		Protocol *protocol)
	{
		Thread::RWLock::RDScoped l(locker_gameservermap);
		GameServerMap::const_iterator it=gameservermap.find(game_id);
		if (it==gameservermap.end()) return false;
		return this->Send((*it).second.sid,protocol);
	}
	bool DispatchProtocol(int game_id,		Protocol &protocol)
	{
		return DispatchProtocol(game_id,&protocol);
	}

	void BroadcastProtocol(const Protocol *protocol)
	{
		Thread::RWLock::RDScoped l(locker_gameservermap);
		GameServerMap::const_iterator it=gameservermap.begin();
		for (;it!=gameservermap.end();it++)
			this->Send((*it).second.sid,protocol);
	}
	void BroadcastProtocol(const Protocol &protocol)
	{
		return BroadcastProtocol(&protocol);
	}
	void BroadcastProtocol(		Protocol *protocol)
	{
		Thread::RWLock::RDScoped l(locker_gameservermap);
		GameServerMap::const_iterator it=gameservermap.begin();
		for (;it!=gameservermap.end();it++)
			this->Send((*it).second.sid,protocol);
	}
	void BroadcastProtocol(		Protocol &protocol)
	{
		return BroadcastProtocol(&protocol);
	}

	//find gameserver by player's position
	static int FindGameServer(const point_t& pt, int worldtag)
	{
		Thread::RWLock::RDScoped	lock(instance.locker_gameservermap);
		for (GameServerMap::iterator it=instance.gameservermap.begin(); it!=instance.gameservermap.end(); it++)
		{
			if ((*it).second.worldtag==worldtag && (*it).second.region.IsEnclosed(pt)) return (*it).first;
		}
		return _GAMESERVER_ID_INVALID;
	}
	//find gameserver by session ID
	int FindGameServer(Session::ID sid)
	{
		Thread::RWLock::RDScoped l(locker_gameservermap);
		GameServerMap::const_iterator it;
		for (it=gameservermap.begin();it!=gameservermap.end();it++)
		{
			if ((*it).second.sid == sid) return (*it).first;
		}
		return _GAMESERVER_ID_INVALID;
	}   

	inline bool InsertPhoneGS(int game_id)
	{
		LOG_TRACE("InsertPhoneGS gameid=%d",game_id);
		Thread::RWLock::WRScoped l(locker_phonegsplayernummap);
		PhoneGSPlayerNumMap::iterator it = phonegsplayernum_map.find(game_id);
		if (it != phonegsplayernum_map.end())
		{
			if ((it->second).isconnect)
				return false;
			(it->second).isconnect = true;
			return true;
		}
		phone_gs_t pgt;
		pgt.isconnect = true;
		pgt.num = 0;
		phonegsplayernum_map.insert(std::make_pair(game_id, pgt));
		return true;
	}
	inline void RemovePhoneGS(int game_id)
	{
		Thread::RWLock::WRScoped l(locker_phonegsplayernummap);
		PhoneGSPlayerNumMap::iterator it = phonegsplayernum_map.find(game_id);
		if (it != phonegsplayernum_map.end())
		{
			(it->second).isconnect = false;
			LOG_TRACE("RemovePhoneGS gameid=%d",game_id);
		}
	}
	inline void AddPhoneGSPlayerNum(int game_id)
	{
		Thread::RWLock::WRScoped l(locker_phonegsplayernummap);
		PhoneGSPlayerNumMap::iterator it = phonegsplayernum_map.find(game_id);
		if (it != phonegsplayernum_map.end())
		{
			(it->second).num ++;
			LOG_TRACE("AddPhoneGSPlayerNum gameid=%d num=%d",game_id,(it->second).num);
		}
	}
	inline void ReducePhoneGSPlayerNum(int game_id)
	{
		Thread::RWLock::WRScoped l(locker_phonegsplayernummap);
		PhoneGSPlayerNumMap::iterator it = phonegsplayernum_map.find(game_id);
		if (it != phonegsplayernum_map.end())
		{
			if ((it->second).num > 0)
			{
				(it->second).num --;
				LOG_TRACE("ReducePhoneGSPlayerNum gameid=%d num=%d",game_id,(it->second).num);
			}
		}
	}
	inline bool GetLessPhoneGS(int & game_id)
	{
		Thread::RWLock::RDScoped l(locker_phonegsplayernummap);
		if (phonegsplayernum_map.empty())
			return false;
		PhoneGSPlayerNumMap::iterator it = phonegsplayernum_map.begin(), eit = phonegsplayernum_map.end();
		PhoneGSPlayerNumMap::iterator nit = it;
		while(!(nit->second).isconnect && nit != eit)
			nit++;
		if (nit == eit)
			return false;
		for ( ; it != eit; ++it)
		{
			if ((it->second).num < (nit->second).num && (it->second).isconnect)
				nit = it;
		}
		game_id = nit->first;
		return true;
	}
	inline bool IsPhoneGS(int game_id)
	{
		Thread::RWLock::RDScoped l(locker_phonegsplayernummap);
		if (phonegsplayernum_map.find(game_id) != phonegsplayernum_map.end())
			return true;
		return false;
	}
};

};
#endif
