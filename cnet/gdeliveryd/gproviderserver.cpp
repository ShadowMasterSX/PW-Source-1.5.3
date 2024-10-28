
#include "gproviderserver.hpp"
#include "state.hxx"
#include "announceproviderid.hpp"
#include "maperaser.h"
#include "mapgameattr.h"
#include "querygameserverattr_re.hpp"
#include "battlemanager.h"
#include "tankbattlemanager.h"

#include "gdeliveryserver.hpp"
#include "gmcontrolgame.hpp"
#include "gmcontrolgame_re.hpp"
#include "forcemanager.h"
#include "globalcontrol.h"

#include "cdcmnfbattleman.h"
#include "cdsmnfbattleman.h"
namespace GNET
{

GProviderServer GProviderServer::instance;

const Protocol::Manager::Session::State* GProviderServer::GetInitState() const
{
	return &state_GProviderDeliveryServer;
}

void GProviderServer::OnAddSession(Session::ID sid)
{
	//TODO
	//announce gameserver attribute to gameserver
	QueryGameServerAttr_Re qgsa_re;
	GameAttrMap::Get(qgsa_re.attr);
	Send(sid,qgsa_re);

	BattleMapNotice pro;
	if(BattleManager::GetInstance()->GetMapNotice(pro))
		Send(sid,pro);
		
	ForceManager::GetInstance()->OnGSConnect(this, sid);
	GlobalControl::GetInstance()->OnGSConnect(this, sid);

	bool is_central = GDeliveryServer::GetInstance()->IsCentralDS();
	if(is_central)
	{
		CDS_MNFactionBattleMan::GetInstance()->GSMNDomainInfoNotice(sid);
	}
	else
	{
		CDC_MNFactionBattleMan::GetInstance()->GSMNDomainInfoNotice(sid);
	}
}

void GProviderServer::OnDelSession(Session::ID sid)
{
	//TODO
	Thread::RWLock::WRScoped l(locker_gameservermap);
	GameServerMap::iterator itg=gameservermap.begin();
	MapEraser<GameServerMap> del_keys(gameservermap);
	for (;itg!=gameservermap.end();itg++)
	{
		if ((*itg).second.sid==sid)
		{
			Log::log(LOG_ERR,"gdelivery::Erase gameserver %d,sid=%d\n",(*itg).first,(*itg).second.sid);
			del_keys.push( (*itg).first );
			RemovePhoneGS((*itg).first);	//在函数里边检测这个gs是否手机登录gs
			TankBattleManager::GetInstance()->DisableServerInfo((*itg).second.worldtag);
		}
	}
}


void GProviderServer::OnGameControlResCheck()
{
	Thread::RWLock::WRScoped lock(locker_gameservermap);
	std::vector<int> remove_dummy;
	for(ControlGameResMap::iterator it = controlgameres_map.begin(), ie = controlgameres_map.end();
			it != ie; ++it)
	{
		ControlGameRes &res = (*it).second;
		++res.tick;
		bool bRemove = true;
		if( res.tick < DEFAULT_GAMECONTROL_TIMEOUT )
		{
			GMControlGameResVector &resvector = res.re->resvector;
			for(GMControlGameResVector::iterator it2 = resvector.begin(), ie2 = resvector.end();
					it2 != ie2; ++it2)
			{
				if( (*it2).retcode == GAMECONTROL_TIMEOUT )
				{
					bRemove = false;
					break;
				}
			}
		}
		if( bRemove )
		{
			remove_dummy.push_back((*it).first);
			GDeliveryServer::GetInstance()->Send(res.client_sid, *res.re);
			delete res.re;
		}
	}
	for(std::vector<int>::const_iterator it = remove_dummy.begin(), ie = remove_dummy.end();
			it != ie; ++it)
	{
		controlgameres_map.erase((*it));
	}
}


void GProviderServer::DeliverGameControl(GMControlGame *gmcg, unsigned int sid)
{
	Thread::RWLock::WRScoped lock(locker_gameservermap);
	GMControlGame_Re* re= new GMControlGame_Re();
	re->xid = gmcg->xid;
	gmcg->xid = ++send_xid;
	for (GameServerMap::iterator it=instance.gameservermap.begin(); it!=instance.gameservermap.end(); it++)
	{
		if ( -1 == gmcg->worldtag || (*it).second.worldtag == gmcg->worldtag ) 
		{
			GMControlGameRes res;
			res.gsid = (*it).first;
			if( Send((*it).second.sid, *gmcg) )
				res.retcode = GAMECONTROL_TIMEOUT;
			else
				res.retcode = GAMECONTROL_SENDFAIL;
			re->resvector.push_back(res);
		}
	}
	if( re->resvector.size() == 0 )
	{
		re->retcode = GAMECONTROL_INVALID_WORLDTAG;
		GDeliveryServer::GetInstance()->Send(sid, *re);
		delete re;
	}
	else
	{
		re->retcode = 0;
		ControlGameRes res;
		res.tick = 0;
		res.client_sid = sid;
		res.re = re;
		controlgameres_map[gmcg->xid] = res;
	}
}
void GProviderServer::DeliverGameControlRes(const GMControlGame_Re *gmcgr, unsigned int sid)
{
	Thread::RWLock::WRScoped lock(locker_gameservermap);
	GameServerMap::const_iterator it;
	for (it=gameservermap.begin();it!=gameservermap.end();it++)
	{
		if ((*it).second.sid == sid)
		{
			int gsid = (*it).first;
			ControlGameResMap::iterator it2 = controlgameres_map.find(gmcgr->xid);
			if( it2 != controlgameres_map.end() )
			{
				GMControlGameResVector &resvector = (*it2).second.re->resvector;
				for(GMControlGameResVector::iterator it3 = resvector.begin(), ie3 = resvector.end();
						it3 != ie3; ++it3)
				{
					if( (*it3).gsid == gsid )
					{
						(*it3).retcode = gmcgr->retcode;
						break;
					}
				}
			}
			return;
		}
	}
}
	

};
