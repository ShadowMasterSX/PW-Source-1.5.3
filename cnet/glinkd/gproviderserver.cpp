
#include "gproviderserver.hpp"
#include "state.hxx"
#include "announceproviderid.hpp"
#include "glinkserver.hpp"
#include "maperaser.h"
namespace GNET
{

GProviderServer GProviderServer::instance;

const Protocol::Manager::Session::State* GProviderServer::GetInitState() const
{
	return &state_GProviderLinkServer;
}

void GProviderServer::OnAddSession(Session::ID sid)
{
}

void GProviderServer::OnDelSession(Session::ID sid)
{
	Thread::RWLock::WRScoped l(locker_gameservermap);
	GameServerMap::iterator itg=gameservermap.begin();
	MapEraser<GameServerMap> del_keys(gameservermap);
	for (;itg!=gameservermap.end();itg++)
	{
		if ((*itg).second==sid)
		{
			Log::log(LOG_ERR,"glinkd::disconnect from gameserver. gameserver %d, sid=%d\n",(*itg).first,(*itg).second);
			Thread::HouseKeeper::AddTimerTask(new DisconnectTimeoutTask((*itg).first),15);
			del_keys.push((*itg).first);
		}
	}
}
void DisconnectTimeoutTask::Run()
{
	//find whether the gameserver is reconnect
	bool blconnect=false;
	{
		GProviderServer* psm=GProviderServer::GetInstance();
		Thread::RWLock::RDScoped l(psm->locker_gameservermap);
		blconnect= psm->gameservermap.find(gid_discon)!=psm->gameservermap.end();
	}
	if (!blconnect) //if connection is not revive,disconnect all players that belongs to this gameserver
	{
		Log::log(LOG_ERR,"glinkd::disconnect from gameserver %d, drop all players\n",gid_discon);
		GLinkServer* gsm=GLinkServer::GetInstance();
		for (GLinkServer::RoleMap::iterator it=gsm->roleinfomap.begin();it!=gsm->roleinfomap.end();++it)
		{
			if ((*it).second.gs_id==gid_discon) 
				gsm->Close((*it).second.sid);
		}
	}
}

};
