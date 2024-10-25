
#include "gfactionclient.hpp"
#include "gproviderserver.hpp"
#include "announceproviderid.hpp"
#include "state.hxx"
#include "timertask.h"
namespace GNET
{

GFactionClient GFactionClient::instance;

void GFactionClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* GFactionClient::GetInitState() const
{
	return &state_GFactionLinkClient;
}

void GFactionClient::OnAddSession(Session::ID sid)
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
	DEBUG_PRINT("glinkd::connect gfactionserver successfully.\n");
	//announce link server's id to gfaction server
	SendProtocol( AnnounceProviderID(GProviderServer::GetProviderServerID()) );
}

void GFactionClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	DEBUG_PRINT("glinkd::gfactionclient: disconnect from server. reconnecting...\n");
}

void GFactionClient::OnAbortSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	DEBUG_PRINT("glinkd::gfactionclient: connect gfactionserver failed. reconnecting...\n");
}

void GFactionClient::OnCheckAddress(SockAddr &sa) const
{
}

};
