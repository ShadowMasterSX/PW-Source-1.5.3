
#include "gfactiondbclient.hpp"
#include "state.hxx"
#include "timertask.h"

namespace GNET
{

GFactionDBClient GFactionDBClient::instance;

void GFactionDBClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	backoff *= 2;
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* GFactionDBClient::GetInitState() const
{
	return &state_GFactionDBClient;
}

void GFactionDBClient::OnAddSession(Session::ID sid)
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
}

void GFactionDBClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
}

void GFactionDBClient::OnAbortSession(const SockAddr &sa)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
}

void GFactionDBClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

};
