
#ifndef __GNET_FACTIONRENAMEGSVERIFY_HPP
#define __GNET_FACTIONRENAMEGSVERIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gsp_if.h"
#include "factionlib.h"
#include "chatbroadcast.hpp"
#include "gproviderclient.hpp"
#include "factionrenamegsverify_re.hpp"

namespace GNET
{

class FactionRenameGsVerify : public GNET::Protocol
{
	#include "factionrenamegsverify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if(step == 0 && retcode == 0) 
		{
			if(!FactionRenameVerify(rid,fid,newname.begin(),newname.size()))		
			{
				manager->Send(sid,FactionRenameGsVerify_Re(rid,fid,ERR_FC_CHECKCONDITION,newname));	
			}
		}
		else if(step == 1 && retcode == 0)
		{
			ChatBroadCast packet;
			packet.msg.swap(newname);
			packet.data.swap(oldname);
			packet.srcroleid = 103;
			packet.channel = GMSV::CHAT_CHANNEL_SYSTEM; 
			GProviderClient::DispatchProtocol(0,&packet);
		}

	}
};

};

#endif
