#ifndef __GNET_GFACTIONCLIENT_HPP
#define __GNET_GFACTIONCLIENT_HPP

#include "protocol.h"
#include "thread.h"

namespace GNET
{

class GFactionClient : public Protocol::Manager
{
	static GFactionClient instance;
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
	//void OnAbortSession(const SockAddr &);
	void OnAbortSession(Session::ID sid);
	void OnCheckAddress(SockAddr &) const;
public:
	static GFactionClient *GetInstance() { return &instance; }
	std::string Identification() const { return "GFactionClient"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GFactionClient() : accumulate_limit(0), conn_state(false), locker_state("GFactionClient::locker_state"),backoff(BACKOFF_INIT) { }

	bool SendProtocol(const Protocol &protocol) { return conn_state && Send(sid, protocol); }
	bool SendProtocol(const Protocol *protocol) { return conn_state && Send(sid, protocol); }
};

};
#endif
