#include "rpcdefs.h"
#include "gamemaster.h"
#include "gdeliveryserver.hpp"
#include "privatechat.hpp"

namespace GNET
{
	void MasterContainer::Insert(int userid, int roleid, int linksid, int localsid, ByteVector& privileges)
	{
		GameMaster master(userid, linksid, localsid, privileges);
		masters.insert(std::make_pair(roleid, master));
		Log::formatlog("gamemaster","login:userid=%d:roleid=%d:linksid=%d:localsid=%d:privileges=%d", 
			userid, roleid, linksid, localsid, privileges.size());
	}

	void MasterContainer::Erase(int roleid)
	{
		masters.erase(roleid);
	}

	bool MasterContainer::Authenticate(int roleid, unsigned int sid, unsigned char right  )
	{
		GameMaster* master = Find(roleid);
		if(sid==GDeliveryServer::GetInstance()->iweb_sid)
			return true;
		if(!master || !master->IsAuthorized(right))
		{
			Log::log(LOG_ERR,"Authentication failed:roleid=%d:sid=%d:right=%d",roleid, sid, right);
			return false;
		}
		return true;
	}
	void MasterContainer::Broadcast(PrivateChat& data, int &roleid)
	{
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		for(MasterMap::iterator it=masters.begin(),ie=masters.end();it!=ie;++it)
		{
			if(!(it->second.status&GameMaster::STATUS_SILENT))
			{
				roleid = it->first;
				dsm->Send(it->second.linksid,data);
			}
		}
	}
	bool MasterContainer::SetSilent(int roleid, bool set)
	{
		GameMaster* master = Find(roleid);
		if(!master)
			return false;
		if(set)
			master->status |= GameMaster::STATUS_SILENT;
		else
			master->status &= (~GameMaster::STATUS_SILENT);
		return true;
	}
}

