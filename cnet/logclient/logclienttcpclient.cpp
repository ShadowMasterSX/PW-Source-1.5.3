
#include "logclienttcpclient.hpp"
#include "state.hxx"
#include "timertask.h"
namespace GNET
{

LogclientTcpClient LogclientTcpClient::instance;

void LogclientTcpClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	backoff *= 2;
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* LogclientTcpClient::GetInitState() const
{
	return &state_LogNull;
}

void LogclientTcpClient::OnAddSession(Session::ID sid)
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
	Log::log( LOG_INFO, "logclienttcp: OnAddSession\n" );
}

void LogclientTcpClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	Log::log( LOG_INFO, "logclienttcp: OnDelSession\n" );
}

void LogclientTcpClient::OnAbortSession(const SockAddr &sa)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	Log::log( LOG_INFO, "logclienttcp: OnAbortSession\n" );
}

void LogclientTcpClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

};
