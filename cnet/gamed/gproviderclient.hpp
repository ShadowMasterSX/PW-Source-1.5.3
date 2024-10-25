#ifndef __GNET_GPROVIDERCLIENT_HPP
#define __GNET_GPROVIDERCLIENT_HPP

#include "protocol.h"
#include "thread.h"
#include "log.h"
#include "keepalive.hpp"
namespace GNET
{

class GProviderClient : public Protocol::Manager
{
	static GProviderClient instance;
	typedef std::map<int,GProviderClient*> ClientArray;
	static Thread::RWLock locker_clientarray;
	static ClientArray clientarray;
	
	size_t		accumulate_limit;
	Session::ID	sid;
	bool		conn_state;
	Thread::Mutex	locker_state;
	enum { BACKOFF_INIT = 2, BACKOFF_DEADLINE = 256 };
	size_t		backoff;
	void Reconnect();
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	void OnAbortSession(Session::ID sid);
	void OnCheckAddress(SockAddr &) const;
	static int m_gameserver_id;	
	static std::string g_edition;
	
	int	m_provider_server_id;
	float left,right,top,bottom;
	int worldtag;
	std::string m_identification;
public:
	static GProviderClient *GetInstance() { return &instance; }
	std::string Identification() const 
	{
		if (!m_identification.empty())
			return m_identification;
		else
	   		return "GProviderClient"; 
	}
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GProviderClient() : accumulate_limit(0), conn_state(false),/*locker_state("GProviderClient::locker_state"),*/ backoff(BACKOFF_INIT) { }
	GProviderClient(std::string id) : accumulate_limit(0), conn_state(false),/*locker_state("GProviderClient::locker_state"),*/backoff(BACKOFF_INIT),m_identification(id) { }
	
	bool SendProtocol(const Protocol &protocol) { return conn_state && Send(sid, protocol); }
	bool SendProtocol(const Protocol *protocol) { return conn_state && Send(sid, protocol); }
	int  GetProviderServerID() { return m_provider_server_id; }
	
	static bool Connect(int gameserver_id,const char* edition, int worldtag, float left=0.0, float right=0.0, float top=0.0, float bottom=0.0);
	static bool Attach(int provider_server_id,GProviderClient* manager)
	{
		Thread::RWLock::WRScoped l(instance.locker_clientarray);
		if (instance.clientarray.find(provider_server_id)!=instance.clientarray.end())
		{
			if (instance.clientarray[provider_server_id]==manager)
				return true;
			else
				return false;
		}
		manager->m_provider_server_id=provider_server_id;
		instance.clientarray[provider_server_id]=manager;
		return true;
	}
	static int GetGameServerID() { return m_gameserver_id; }
	static bool DispatchProtocol(int provider_server_id,const Protocol &protocol);
	static bool DispatchProtocol(int provider_server_id,const Protocol *protocol);	
	static bool DispatchProtocol(int provider_server_id,	  Protocol &protocol);
	static bool DispatchProtocol(int provider_server_id,	  Protocol *protocol);	

	static void BroadcastProtocol(const Protocol &protocol);
	static void BroadcastProtocol(const Protocol *protocol);
	static void BroadcastProtocol(		Protocol &protocol);
	static void BroadcastProtocol(		Protocol *protocol);

};
class KeepAliveTask : public Thread::Runnable
{
	unsigned int delay;
public:
	KeepAliveTask(unsigned int _delay,unsigned int priority=1) : Runnable(priority),delay(_delay) { } 
	~KeepAliveTask() { }
	void Run()
	{
		GProviderClient::BroadcastProtocol(KeepAlive((unsigned int)PROTOCOL_KEEPALIVE));
		Thread::HouseKeeper::AddTimerTask(this,delay);
		PollIO::WakeUp();
	}	
};

};
#endif
