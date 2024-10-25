
#include "gwebtradeclient.hpp"
#include "state.hxx"
#include "timertask.h"

#include "gdeliveryserver.hpp"
#include "announcezoneid2.hpp"

namespace GNET
{

GWebTradeClient GWebTradeClient::instance;

void GWebTradeClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	backoff *= 2;
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* GWebTradeClient::GetInitState() const
{
	return &state_GWebTradeClient;
}

void GWebTradeClient::OnAddSession(Session::ID sid)
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
	DEBUG_PRINT("gwebtradeclient:: connect Web Trade Server successfully!");
	
	SendProtocol( AnnounceZoneid2((unsigned char)GDeliveryServer::GetInstance()->zoneid,GDeliveryServer::GetInstance()->aid) );
}

void GWebTradeClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	DEBUG_PRINT("gwebtradeclient:: disconnect from Web Trade Server!");
}

void GWebTradeClient::OnAbortSession(const SockAddr &sa)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	DEBUG_PRINT("gwebtradeclient:: connect to Web Trade Server failed\n");
}

void GWebTradeClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

};
