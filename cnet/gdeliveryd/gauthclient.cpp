
#include "gauthclient.hpp"
#include "state.hxx"
#include "timertask.h"
#include "announcezoneid3.hpp"
#include "gdeliveryserver.hpp"
namespace GNET
{

GAuthClient GAuthClient::instance;

void GAuthClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	//backoff *= 2;
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}
void GAuthClient::OnSetTransport(Session::ID sid, const SockAddr& local, const SockAddr& peer)
{
	
}

const Protocol::Manager::Session::State* GAuthClient::GetInitState() const
{
	return &state_GAuthClient;
}

void GAuthClient::OnAddSession(Session::ID sid)
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

	bl_keepalive = true;
	//Send AnnounceZoneid3 to gauthd
	SendProtocol(AnnounceZoneid3((unsigned char)GDeliveryServer::GetInstance()->zoneid,
				GDeliveryServer::GetInstance()->aid,blreset,0,0,0,1,0));
	if (blreset) blreset=false; //only reset authserver once
	
	
	DEBUG_PRINT("gdelivery::connect to gauthd successfully.\n");
}

void GAuthClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	bl_keepalive = false;
	
	if(identify_failed) return;
	Reconnect();
	//TODO
	Log::log(LOG_ERR,"gdelivery::disconnect from GAuth server. Reconnecting....\n");
}

void GAuthClient::OnAbortSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	bl_keepalive = false;
	
	if(identify_failed) return;
	Reconnect();
	//TODO
	Log::log(LOG_ERR,"gdelivery::connect to GAuth server failed. Reconnecting....\n");
}

void GAuthClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

bool GAuthClient::SendProtocol(const Protocol *protocol)
{
	if(bl_keepalive || (protocol->GetType() == PROTOCOL_CERTREQUEST || protocol->GetType() == PROTOCOL_CERTKEY))
	{
		return conn_state && Send(sid, protocol); 
	}
	else
	{
		return false;
	}
}

bool GAuthClient::SendProtocol(		Protocol *protocol)
{
	if(bl_keepalive || (protocol->GetType() == PROTOCOL_CERTREQUEST || protocol->GetType() == PROTOCOL_CERTKEY))
	{
		return conn_state && Send(sid, protocol); 
	}
	else
	{
		return false;
	}
}

void GAuthClient::IdentifyFailed()
{
	identify_failed = true;
	Close(sid);
}
};
