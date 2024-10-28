
#include "centraldeliveryclient.hpp"
#include "state.hxx"
#include "timertask.h"
#include "gdeliveryserver.hpp"
#include "dsannounceidentity.hpp"
#include "mapuser.h"

namespace GNET
{

CentralDeliveryClient CentralDeliveryClient::instance;

void CentralDeliveryClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	//backoff *= 2;
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* CentralDeliveryClient::GetInitState() const
{
	return &state_CentralDeliveryClient;
}

class DelayDSAnnounceTask : public Thread::Runnable
{
	Protocol::Manager::Session::ID _sid;
	unsigned int _delay;
public:
	DelayDSAnnounceTask(Protocol::Manager::Session::ID sid, unsigned int delay, unsigned int priority=1) 
			: Runnable(priority), _sid(sid), _delay(delay){}

	void Run()
	{
		CentralDeliveryClient * cdc = CentralDeliveryClient::GetInstance();
		if(cdc->ValidSid(_sid))
		{
			GDeliveryServer* dsm = GDeliveryServer::GetInstance();
			int zoneid = (unsigned char)dsm->zoneid;
			unsigned int version = dsm->GetVersion();
			Octets edition = dsm->GetEdition();
			if(version != 0 && edition.size() != 0)
			{
				cdc->SendProtocol(DSAnnounceIdentity(zoneid, version, edition));
				DEBUG_PRINT("CrossRelated Try to Connect to central delivery, zoneid %d version %d edition.size %d", zoneid, version, edition.size());
				delete this;
			}
			else
			{
				Thread::HouseKeeper::AddTimerTask(this,_delay);
				DEBUG_PRINT("CrossRelated CentralDeliveryClient version %d edition.size %d not ready, wait...", version, edition.size());
			}
		}
		else
		{
			delete this;	
		}
	}	
};

void CentralDeliveryClient::OnAddSession(Session::ID sid)
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
	
	Thread::HouseKeeper::AddTimerTask(new DelayDSAnnounceTask(sid, 5), 5);
	/*
	GDeliveryServer* dsm = GDeliveryServer::GetInstance();
	int zoneid = (unsigned char)dsm->zoneid;
	unsigned int version = dsm->GetVersion();
	Octets edition = dsm->GetEdition();
	
	if (version != 0 && edition.size() != 0) {
		SendProtocol(DSAnnounceIdentity(zoneid, version, edition));
		Log::log(LOG_ERR, "CrossRelated Try to Connect to central delivery, zoneid %d version %d edition.size %d", zoneid, version, edition.size());
	} else {
		Log::log(LOG_ERR, "CrossRelated CentralDeliveryClient version %d edition.size %d not ready, disconnect connection", version, edition.size());
		Close(sid);
		return;
	}*/
}

void CentralDeliveryClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();

	cache_server_load.ClearLoad();
	int count = UserContainer::GetInstance().ClearRemoteUsers();

	Log::log(LOG_ERR, "CrossRelated Disconnect from central delivery, clear %d users", count);
}

void CentralDeliveryClient::OnAbortSession(const SockAddr &sa)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	Log::log(LOG_ERR, "CrossRelated Connect to central delivery failed");
}

void CentralDeliveryClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

void CentralDeliveryClient::OnSetTransport(Session::ID sid, const SockAddr& local, const SockAddr& peer)
{
	server_addr = peer;
}

};
