
#ifndef __GNET_FACTIONACCEPTJOIN_HPP
#define __GNET_FACTIONACCEPTJOIN_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "ids.hxx"
#include "guserfaction"
#include "factiondb.h"

#include "gfactionserver.hpp"
#include "factionacceptjoin_re.hpp"
#include "gfs_io.h"
#include "factiondb.h"
namespace GNET
{

class FactionAcceptJoin : public GNET::Protocol
{
	#include "factionacceptjoin"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//check whether roleid is a faction officer
		GFactionServer::Player inviter,invitee;
		if (!GFactionServer::GetInstance()->GetPlayer(roleid,inviter))
			return;
		if (!GFactionServer::GetInstance()->GetPlayer(invited_roleid,invitee))
		{
			//invited role is not online
			gfs_send_factionacceptjoin_re(roleid,ERR_FC_OFFLINE,invited_roleid,roleid);
			return;
		}
		if ( inviter.faction_id && inviter.froleid != _R_MEMBER && invitee.froleid == _R_UNMEMBER)
		{
			//send rpc
			FactionBriefInfo info;
			if ( Factiondb::GetInstance()->FindFactionInCache(info) )
				gfs_send_factioninvitejoin(roleid,invited_roleid,inviter.faction_id,info.name,inviter.name);
			else
				gfs_send_factioninvitejoin(roleid,invited_roleid,inviter.faction_id,Octets(),inviter.name);
		}
		else
		{
			//send err
			int err=0;
			if (inviter.faction_id==0||inviter.froleid==_R_MEMBER) 
				err=ERR_FC_NO_PRIVILEGE;
			else if (invitee.froleid!=_R_UNMEMBER) 
				err=ERR_FC_ACCEPT_REACCEPT;
			
			gfs_send_factionacceptjoin_re(roleid,err,invited_roleid,roleid);
		}
	}
};

};

#endif
