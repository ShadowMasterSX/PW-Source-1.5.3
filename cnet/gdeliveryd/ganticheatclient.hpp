#ifndef __GNET_GANTICHEATCLIENT_HPP
#define __GNET_GANTICHEATCLIENT_HPP

#include "protocol.h"
#include "thread.h"

namespace GNET
{

class GAntiCheatClient : public Protocol::Manager
{
	static GAntiCheatClient instance;
	size_t		accumulate_limit;
	Session::ID	sid;
	bool		conn_state;
	Thread::Mutex	locker_state;
	enum { BACKOFF_INIT = 2, BACKOFF_DEADLINE = 64 };
	size_t		backoff;
	void Reconnect();
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	void OnAbortSession(const SockAddr &sa);
	void OnCheckAddress(SockAddr &) const;
public:
	static GAntiCheatClient *GetInstance() { return &instance; }
	std::string Identification() const { return "GAntiCheatClient"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GAntiCheatClient() : accumulate_limit(0), conn_state(false),locker_state("GAntiCheatClient::locker_state"), backoff(BACKOFF_INIT) { }

	bool SendProtocol(const Protocol &protocol) { return conn_state && Send(sid, protocol); }
	bool SendProtocol(const Protocol *protocol) { return conn_state && Send(sid, protocol); }
};

};
#endif
