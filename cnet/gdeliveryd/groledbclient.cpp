
#include "groledbclient.hpp"
#include "state.hxx"
#include "timertask.h"
namespace GNET
{

GRoleDBClient GRoleDBClient::instance;

void GRoleDBClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	backoff *= 2;
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* GRoleDBClient::GetInitState() const
{
	return &state_GRoleDBClient;
}

void GRoleDBClient::OnAddSession(Session::ID sid)
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
	printf("gdelivery::connect RoleDB successfully.\n");
}

void GRoleDBClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	printf("gdelivery::disconnect from RoleDB.\n");
}

void GRoleDBClient::OnAbortSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	printf("gdelivery::connect RoleDB failed. Reconnecting....\n");
}

void GRoleDBClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

};
