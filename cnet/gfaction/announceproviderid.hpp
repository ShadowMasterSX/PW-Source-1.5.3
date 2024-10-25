
#ifndef __GNET_ANNOUNCEPROVIDERID_HPP
#define __GNET_ANNOUNCEPROVIDERID_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gfactionserver.hpp"
#include "gproviderserver.hpp"
namespace GNET
{

class AnnounceProviderID : public GNET::Protocol
{
	#include "announceproviderid"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		// if manager is gfactionserver, then client is linkserver or delivery server
		if (manager == GFactionServer::GetInstance())
		{
			GFactionServer* gfs=(GFactionServer*) manager;
			if (!gfs->RegisterLink(id,sid))
			{
				Log::log(LOG_ERR,"gfactionserver:: identical linkserver(or delivery) id(%d) find. check .conf file.\n",id);
				gfs->Close(sid);
				return;
			}
			DEBUG_PRINT("gfactionserver:: add link(or Delivery) %d to map.\n",id);
		}
		//if manager is providerserver, then client is gameserver
		else if (manager == GProviderServer::GetInstance())
		{
			//register gameserver
			GProviderServer* gps=(GProviderServer*) manager;
			if (!gps->RegisterGameServer(id,sid))
			{
				Log::log(LOG_ERR,"gproviderserver:: identical gameserver id(%d) find. check .conf file.\n",id);
				gps->Close(sid);
				return;
			}
			DEBUG_PRINT("gproviderserver:: add gameserver %d to map.\n",id);
			//announce providerserver's id to gameserver
			this->id=gps->GetProviderID();
			gps->Send(sid,this);
		}
	}
};

};

#endif
