
#ifndef __GNET_FACTIONCHAT_HPP
#define __GNET_FACTIONCHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#define _FACTIONSERVER_ID 101

size_t handle_chatdata(int uid, const void * aux_data, size_t size, void * buffer, size_t len);
namespace GNET
{

class FactionChat : public GNET::Protocol
{
	#include "factionchat"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if(data.size())
		{
			char buffer[1024];
			int size = handle_chatdata(src_roleid, data.begin(), data.size(), buffer , sizeof(buffer));
			data.clear();
			if(size > 0) data.insert(data.begin(), buffer, size);
		}
		GProviderClient::GetInstance()->DispatchProtocol(_FACTIONSERVER_ID, this);
	}
};

};

#endif
