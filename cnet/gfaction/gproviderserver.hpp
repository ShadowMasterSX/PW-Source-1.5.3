#ifndef __GNET_GPROVIDERSERVER_HPP
#define __GNET_GPROVIDERSERVER_HPP

#include "protocol.h"
#include "macros.h"
namespace GNET
{

class GProviderServer : public Protocol::Manager
{
	static GProviderServer instance;
	size_t		accumulate_limit;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);

	int id; //provider server's id
public:
	static GProviderServer *GetInstance() { return &instance; }
	std::string Identification() const { return "GProviderServer"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GProviderServer() : accumulate_limit(0),
						id(_PROVIDER_ID_INVALID),
						locker_gameservermap("GProviderServer::GameserverMap") 
	{ }
	
	//get,set provider id
	unsigned int GetProviderID() { return id; }
	void SetProviderID(unsigned int _id) { id=_id; }

	//gameserver id --> session id map
	Thread::RWLock locker_gameservermap;
	typedef std::map<int,Session::ID> GameServerMap;
	GameServerMap gameservermap;

	//gameserver management
	bool RegisterGameServer(int game_id, Session::ID sid)
	{
		Thread::RWLock::WRScoped l(locker_gameservermap);
		GameServerMap::iterator it=gameservermap.find(game_id);
		if (it!=gameservermap.end()) return false; //出现游戏服务器ID冲突
		gameservermap[game_id]=sid;
		return true;
	}
	void UnRegisterGameServer(int game_id)
	{
		Thread::RWLock::WRScoped l(locker_gameservermap);
		gameservermap.erase(game_id);
	}
	void UnRegisterSession(Session::ID sid)
	{
		Thread::RWLock::WRScoped l(locker_gameservermap);
		for (GameServerMap::iterator it=gameservermap.begin();it!=gameservermap.end();it++)
		{
			if ((*it).second==sid)
			{
				printf("gfactionserver::ondelsession:erase gameserver(%d) from map.\n",(*it).first);
				gameservermap.erase(it);
				return;
			}
		}
	}
	//send protocol
	bool DispatchProtocol(int game_id,const Protocol *protocol)
	{
		Thread::RWLock::RDScoped l(locker_gameservermap);
		GameServerMap::const_iterator it=gameservermap.find(game_id);
		if (it==gameservermap.end()) return false;
		return this->Send((*it).second,protocol);
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
		return this->Send((*it).second,protocol);
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
			this->Send((*it).second,protocol);
	}
	void BroadcastProtocol(const Protocol &protocol)
	{
		return BroadcastProtocol(&protocol);
	}

};

};
#endif
