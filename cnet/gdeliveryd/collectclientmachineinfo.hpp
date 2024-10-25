
#ifndef __GNET_COLLECTCLIENTMACHINEINFO_HPP
#define __GNET_COLLECTCLIENTMACHINEINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mapuser.h"
#include "game2au.hpp"
#include "gauthclient.hpp"


namespace GNET
{

class CollectClientMachineInfo : public GNET::Protocol
{
	#include "collectclientmachineinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
        UserInfo* userinfo = UserContainer::GetInstance().FindUser(userid);
        if (userinfo == NULL)
            return;

        Game2AU proto;
        proto.userid = userid;
        proto.qtype = Game2AU::COLLECT_CLIENT_MACHINE_INFO;
        proto.info = (Marshal::OctetsStream() << (userinfo->ip) << machineinfo << (int)0 << Octets());
        proto.reserved = 0;

        GAuthClient::GetInstance()->SendProtocol(proto);
	}
};

};

#endif
