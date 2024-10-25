
#include "snsclient.hpp"
#include "state.hxx"
#include "timertask.h"
#include "announcezoneidtoim.hpp"
#include "gdeliveryserver.hpp"
namespace GNET
{

SNSClient SNSClient::instance;

void SNSClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	backoff *= 2;
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* SNSClient::GetInitState() const
{
	return &state_SNSClient;
}

void SNSClient::OnAddSession(Session::ID sid)
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
	DEBUG_PRINT("snsclient:: connect to SNS Server successfully!");
	SendProtocol( AnnounceZoneidToIM(GDeliveryServer::GetInstance()->aid,(unsigned char)GDeliveryServer::GetInstance()->zoneid,0) );
}

void SNSClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	DEBUG_PRINT("snsclient:: disconnect from SNS Server!");
}

void SNSClient::OnAbortSession(const SockAddr &sa)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	DEBUG_PRINT("snsclient:: connect to SNS Server failed\n");
}

void SNSClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

};
