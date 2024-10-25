
#include "gamedbclient.hpp"
#include "state.hxx"
#include "timertask.h"
namespace GNET
{

GamedbClient GamedbClient::instance;

void GamedbClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	backoff *= 2;
	if (backoff > 32) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* GamedbClient::GetInitState() const
{
	return &state_GameDBClient;
}

void GamedbClient::OnAddSession(Session::ID sid)
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
}

void GamedbClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
}

void GamedbClient::OnAbortSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
}

void GamedbClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

};
