
#include "gdeliveryclient.hpp"
#include "state.hxx"
#include "timertask.h"
#include "announcelinkversion.hpp"
#include "announcelinktype.hpp"
#include "glinkserver.hpp"
#include "glinkserver.hpp"
#include "localmacro.h"

namespace GNET
{

GDeliveryClient GDeliveryClient::instance;

void GDeliveryClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* GDeliveryClient::GetInitState() const
{
	return &state_GDeliverClient;
}

void GDeliveryClient::OnAddSession(Session::ID sid)
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

	AnnounceLinkVersion pro(GLinkServer::GetInstance()->version);
	SendProtocol(pro);

	AnnounceLinkType altpro(link_type);
	SendProtocol(altpro);
	LOG_TRACE("GDeliveryClient::OnAddSession link_type=%d",link_type);
}

void GDeliveryClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	Log::log(LOG_ERR,"glinkd::Disconnect from delivery.\n");
}

void GDeliveryClient::OnAbortSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	Log::log(LOG_ERR,"glinkd::Connect to delivery failed.\n");
}

void GDeliveryClient::OnCheckAddress(SockAddr &sa) const
{
}

};
