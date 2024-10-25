
#include "gproviderclient.hpp"
#include "state.hxx"
#include "timertask.h"
#include "conf.h"
#include "log.h"
#include "announceproviderid.hpp"
namespace GNET
{
int 							GProviderClient::m_gameserver_id;	
GProviderClient 				GProviderClient::instance;
GProviderClient::ClientArray 	GProviderClient::clientarray;
//Thread::RWLock					GProviderClient::locker_clientarray("gamed::locker_clientarray");
Thread::RWLock					GProviderClient::locker_clientarray;
std::string GProviderClient::g_edition;

bool GProviderClient::Connect(int gameserver_id, const char * edition, int worldtag, float left, float right, float top, float bottom)
{
	/*
	if (0==(instance.m_gameserver_id=std::atoi( Conf::GetInstance()->find( "GameServerID" , string(GetHostIP()) ).c_str() )))
	{
		Log::log(LOG_ERR,"gamed :: require m_gameserver_id error. Maybe someting wrong with .conf file");
		return false;
	}
	*/
	instance.m_gameserver_id = gameserver_id;
	instance.left = left;
	instance.right = right;
	instance.top = top;
	instance.bottom = bottom;
	instance.worldtag=worldtag;
	g_edition =  edition;
	
	int count=std::atoi( Conf::GetInstance()->find( "ProviderServers" , "count" ).c_str() );
	if (count<3)
	{
		Log::log(LOG_ERR,"gamed :: Too few provider servers( should >=3). Maybe someting wrong with .conf file");
	   	return false;
	}
	char buf[128];
	for (;count--;)
	{
		sprintf(buf,"%d",count);
		GProviderClient* manager=new GProviderClient(instance.Identification()+string(buf));
		DEBUG_PRINT("gamed:: start to connect %s\n",manager->Identification().c_str());
		printf("gamed:: start to connect %s\n",manager->Identification().c_str());
		Protocol::Client(manager);
		//manager->m_provider_server_id=std::atoi( Conf::GetInstance()->find(manager->Identification(),"id").c_str() );
		manager->SetAccumulate(atoi(Conf::GetInstance()->find(manager->Identification(), "accumulate").c_str()));
		/*
		Thread::RWLock::WRScoped l(instance.locker_clientarray);
		instance.clientarray[manager->m_provider_server_id]=manager;
		*/
	}
	Thread::HouseKeeper::AddTimerTask(new KeepAliveTask(30),0); 
	PollIO::WakeUp();
	return true;
}
bool GProviderClient::DispatchProtocol(int provider_server_id,const Protocol &protocol)
{
	return DispatchProtocol(provider_server_id,&protocol);
}
bool GProviderClient::DispatchProtocol(int provider_server_id,const Protocol *protocol)
{
	Thread::RWLock::RDScoped l(instance.locker_clientarray);
	GProviderClient::ClientArray::const_iterator it=instance.clientarray.find(provider_server_id);
	if (it==instance.clientarray.end()) return false;
	return (*it).second->SendProtocol(protocol);
}
bool GProviderClient::DispatchProtocol(int provider_server_id,		Protocol &protocol)
{
	return DispatchProtocol(provider_server_id,&protocol);
}
bool GProviderClient::DispatchProtocol(int provider_server_id,		Protocol *protocol)
{
	Thread::RWLock::RDScoped l(instance.locker_clientarray);
	GProviderClient::ClientArray::const_iterator it=instance.clientarray.find(provider_server_id);
	if (it==instance.clientarray.end()) return false;
	return (*it).second->SendProtocol(protocol);
}	
	
void GProviderClient::BroadcastProtocol(const Protocol &protocol)
{
	BroadcastProtocol(&protocol);
}
void GProviderClient::BroadcastProtocol(const Protocol *protocol)
{
	Thread::RWLock::WRScoped l(instance.locker_clientarray);
	GProviderClient::ClientArray::const_iterator it=instance.clientarray.begin();
	for (;it!=instance.clientarray.end();it++)
	{
		(*it).second->SendProtocol(protocol);
	}
}
void GProviderClient::BroadcastProtocol(	 Protocol &protocol)
{
	BroadcastProtocol(&protocol);
}
void GProviderClient::BroadcastProtocol(	 Protocol *protocol)
{
	Thread::RWLock::WRScoped l(instance.locker_clientarray);
	GProviderClient::ClientArray::const_iterator it=instance.clientarray.begin();
	for (;it!=instance.clientarray.end();it++)
	{
		(*it).second->SendProtocol(protocol);
	}
}	
	
void GProviderClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	//backoff *= 2;
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* GProviderClient::GetInitState() const
{
	return &state_GProviderClient;
}

void GProviderClient::OnAddSession(Session::ID sid)
{
	DEBUG_PRINT("gamed(%d):: OnAddSession %d\n",GetGameServerID(),sid);
	printf("gamed(%d):: OnAddSession %d\n",GetGameServerID(),sid);
	Thread::Mutex::Scoped l(locker_state);
	if (conn_state)
	{
		Close(sid);
		return;
	}
	conn_state = true;
	this->sid = sid;
	backoff = BACKOFF_INIT;
	SendProtocol( AnnounceProviderID(GetGameServerID(),GetInstance()->left,GetInstance()->right,GetInstance()->top,
		GetInstance()->bottom,GetInstance()->worldtag, Octets(g_edition.c_str(), g_edition.size())));
	PollIO::WakeUp();
}

void GProviderClient::OnDelSession(Session::ID sid)
{
	DEBUG_PRINT("gamed(%d):: OnDelSession %d\n",GetGameServerID(),sid);
	printf("gamed(%d):: OnDelSession %d\n",GetGameServerID(),sid);

	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
}

void GProviderClient::OnAbortSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	Reconnect();
	//TODO
	DEBUG_PRINT("gamed(%d):: OnAbortSession %d\n",GetGameServerID(),sid);
	printf("gamed(%d):: OnAbortSession %d\n",GetGameServerID(),sid);
}

void GProviderClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

};
