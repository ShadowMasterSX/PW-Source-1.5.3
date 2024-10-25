
#ifndef __GNET_CHATROOMINVITE_RE_HPP
#define __GNET_CHATROOMINVITE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "mapuser.h"
namespace GNET
{

class ChatRoomInvite_Re : public GNET::Protocol
{
	#include "chatroominvite_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(inviter>0 && invitee>0)
		{
			GDeliveryServer* dsm = GDeliveryServer::GetInstance();
			Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( (inviter) );
			if (NULL != pinfo )
			{
				localsid = pinfo->localsid;
				dsm->Send(pinfo->linksid,this);
				return;
			}
		}
	}
};

};

#endif
