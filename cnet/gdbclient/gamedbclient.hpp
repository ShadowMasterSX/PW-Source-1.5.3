#ifndef __GNET_GAMEDBCLIENT_HPP
#define __GNET_GAMEDBCLIENT_HPP

#include "protocol.h"
#include "thread.h"

namespace GNET
{

class GamedbClient : public Protocol::Manager
{
	static GamedbClient instance;
	size_t		accumulate_limit;
	Session::ID	sid;
	bool		conn_state;
	Thread::Mutex	locker_state;
	enum { BACKOFF_INIT = 4, BACKOFF_DEADLINE = 32 };
	size_t		backoff;
	void Reconnect();
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	void OnAbortSession(Session::ID sid);
	void OnCheckAddress(SockAddr &) const;
public:
	static GamedbClient *GetInstance() { return &instance; }
	std::string Identification() const { return "GamedbClient"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GamedbClient() : accumulate_limit(0), conn_state(false), backoff(BACKOFF_INIT) { }

	bool SendProtocol(Protocol &protocol) { return conn_state && Send(sid, protocol); }
	bool SendProtocol(Protocol *protocol) { return conn_state && Send(sid, protocol); }
};

};
#endif
