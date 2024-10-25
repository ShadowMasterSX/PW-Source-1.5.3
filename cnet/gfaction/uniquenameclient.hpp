#ifndef __GNET_UNIQUENAMECLIENT_HPP
#define __GNET_UNIQUENAMECLIENT_HPP

#include "protocol.h"
#include "thread.h"
#include "keepalive.hpp"

namespace GNET
{

class UniqueNameClient : public Protocol::Manager
{
	static UniqueNameClient instance;
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
	void OnAbortSession(const SockAddr &sa);
	void OnCheckAddress(SockAddr &) const;
public:
	bool IsConnect() { Thread::Mutex::Scoped l(locker_state); return conn_state; }
	static UniqueNameClient *GetInstance() { return &instance; }
	std::string Identification() const { return "UniqueNameClient"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	UniqueNameClient() : accumulate_limit(0), conn_state(false), locker_state("UniqueNameClient::locker_state"), backoff(BACKOFF_INIT) { }

	bool SendProtocol(const Protocol &protocol) { return conn_state && Send(sid, protocol); }
	bool SendProtocol(const Protocol *protocol) { return conn_state && Send(sid, protocol); }
	bool SendProtocol(		Protocol &protocol) { return conn_state && Send(sid, protocol); }
	bool SendProtocol(		Protocol *protocol) { return conn_state && Send(sid, protocol); }
};

class KeepAliveTask : public Thread::Runnable
{
	unsigned int delay;
public:
	KeepAliveTask(unsigned int _delay,unsigned int priority=1) : Runnable(priority),delay(_delay) { } 
	~KeepAliveTask() { }
	void Run()
	{
		UniqueNameClient::GetInstance()->SendProtocol(KeepAlive((unsigned int)PROTOCOL_KEEPALIVE));
		Thread::HouseKeeper::AddTimerTask(this,delay);
		PollIO::WakeUp();
	}	
};
};
#endif
