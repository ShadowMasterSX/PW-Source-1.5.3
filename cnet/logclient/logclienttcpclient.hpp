#ifndef __GNET_LOGCLIENTTCPCLIENT_HPP
#define __GNET_LOGCLIENTTCPCLIENT_HPP

#include "protocol.h"
#include "thread.h"

namespace GNET
{

class LogclientTcpClient : public Protocol::Manager
{
	static LogclientTcpClient instance;
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
	static LogclientTcpClient *GetInstance() { return &instance; }
	std::string Identification() const { return "LogclientTcpClient"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	LogclientTcpClient() : accumulate_limit(0), conn_state(false), locker_state("LogclientTcpClient::locker_state"), backoff(BACKOFF_INIT) { }

	bool SendProtocol(const Protocol &protocol) { return conn_state && Send(sid, protocol); }
	bool SendProtocol(const Protocol *protocol) { return conn_state && Send(sid, protocol); }
};

};
#endif
