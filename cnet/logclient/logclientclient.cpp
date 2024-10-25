
#include "logclientclient.hpp"
#include "state.hxx"
#include "timertask.h"

#include "log.h"

namespace GNET
{

LogclientClient LogclientClient::instance;

void LogclientClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	backoff *= 2;
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* LogclientClient::GetInitState() const
{
	return &state_LogNull;
}

void LogclientClient::OnAddSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	if (conn_state)
	{
		Close(sid);
		return;
	}
	conn_state = true;
	this->sid = sid;
	backoff = BACKOFF_INIT;
	//TODO
	Log::log( LOG_INFO, "logclient: OnAddSession\n" );
}

void LogclientClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	Log::log( LOG_INFO, "logclient: OnDelSession\n" );
}

void LogclientClient::OnAbortSession(const SockAddr &sa)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	Log::log( LOG_INFO, "logclient: OnAbortSession\n" );
}

void LogclientClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

};
