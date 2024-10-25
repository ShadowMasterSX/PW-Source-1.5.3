
#include "gametalkclient.hpp"
#include "state.hxx"
#include "timertask.h"
#include "gametalkmanager.h"
#include "rpcdefs.h"
#include "time.h"

namespace GNET
{

GameTalkClient GameTalkClient::instance;

void GameTalkClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	backoff *= 2;
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* GameTalkClient::GetInitState() const
{
	return &state_GameTalkClient;
}

void GameTalkClient::OnAddSession(Session::ID sid)
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
	GameTalkManager::GetInstance()->OnConnected();
}

// should call the later version
void GameTalkClient::OnDelSession(Session::ID sid)
{
	/* 
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	GameTalkManager::GetInstance()->OnDisconnected();
	Reconnect();
	*/
}

void GameTalkClient::OnDelSession(Session::ID sid, int status)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	GameTalkManager::GetInstance()->OnDisconnected();
	if(status != CLOSE_ACTIVE) Reconnect();
}

void GameTalkClient::OnAbortSession(const SockAddr &sa)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	DEBUG_PRINT("GameTalkClient: connect fail.");
	Reconnect();
}

void GameTalkClient::OnCheckAddress(SockAddr &sa) const
{
}
};
