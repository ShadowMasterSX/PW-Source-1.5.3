#ifndef __GNET_GAMETALKCLIENT_HPP
#define __GNET_GAMETALKCLIENT_HPP

#include "protocol.h"
#include "thread.h"
#include <vector>

namespace GNET
{

class GameTalkClient : public Protocol::Manager
{
	static GameTalkClient instance;
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
	void OnDelSession(Session::ID sid, int status);
	void OnAbortSession(const SockAddr &sa);
	void OnCheckAddress(SockAddr &) const;
public:
	static GameTalkClient *GetInstance() { return &instance; }
	std::string Identification() const { return "GameTalkClient"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GameTalkClient() : accumulate_limit(0), conn_state(false), locker_state("GameTalkClient::locker_state"), backoff(BACKOFF_INIT) { }

	bool SendProtocol(const Protocol &protocol) { return conn_state && Send(sid, protocol); }
	bool SendProtocol(const Protocol *protocol) { return conn_state && Send(sid, protocol); }

	void ActiveClose() { Close(sid); }
};

};
#endif
