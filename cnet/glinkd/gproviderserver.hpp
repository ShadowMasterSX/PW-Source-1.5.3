#ifndef __GNET_GPROVIDERSERVER_HPP
#define __GNET_GPROVIDERSERVER_HPP

#include "protocol.h"
#include "macros.h"
#include "thread.h"
namespace GNET
{
class DisconnectTimeoutTask : public Thread::Runnable
{
	int gid_discon;  //gameserver id that is disconnect from link
public:
	DisconnectTimeoutTask(int _gid) : Runnable(1),gid_discon(_gid) { }
	~DisconnectTimeoutTask() { }
	void Run();
};

class GProviderServer : public Protocol::Manager
{
	static GProviderServer instance;
	size_t		accumulate_limit;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);

	int m_provider_server_id;

	string section_num;
public:
	static GProviderServer *GetInstance() { return &instance; }
	void SetSectionNum(const char* sn) { section_num=sn; }
	std::string Identification() const { return string("GProviderServer")+section_num; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GProviderServer() : 
		accumulate_limit(0),
		m_provider_server_id(_PROVIDER_ID_INVALID),
		locker_gameservermap("GProviderServer::locker_gameservermap")
   	{ }

	Thread::RWLock locker_gameservermap;
	typedef std::map<int,Session::ID> GameServerMap;
	GameServerMap gameservermap;
	void SetProviderServerID(int ps_id) { m_provider_server_id=ps_id; } 
	static int GetProviderServerID() { return instance.m_provider_server_id; } 

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
	void BroadcastProtocol(		Protocol *protocol)
	{
		Thread::RWLock::RDScoped l(locker_gameservermap);
		GameServerMap::const_iterator it=gameservermap.begin();
		for (;it!=gameservermap.end();it++)
			this->Send((*it).second,protocol);
	}
	void BroadcastProtocol(		Protocol &protocol)
	{
		return BroadcastProtocol(&protocol);
	}

	int FindGameServer(Session::ID sid)
	{
		Thread::RWLock::RDScoped l(locker_gameservermap);
		GameServerMap::const_iterator it;
		for (it=gameservermap.begin();it!=gameservermap.end();it++)
		{
			if ((*it).second == sid) return (*it).first;
		}
		return _GAMESERVER_ID_INVALID;
	}	
};

};
#endif
