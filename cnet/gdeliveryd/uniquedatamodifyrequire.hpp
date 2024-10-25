
#ifndef __GNET_UNIQUEDATAMODIFYREQUIRE_HPP
#define __GNET_UNIQUEDATAMODIFYREQUIRE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "uniquedataserver.h"

namespace GNET
{

class UniqueDataModifyRequire : public GNET::Protocol
{
	#include "uniquedatamodifyrequire"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if(vtype)
			UniqueDataServer::GetInstance()->ModifyUniqueData(worldtag,key,vtype,value,oldvalue,exclusive,broadcast,sid,version,timeout);
		else
			UniqueDataServer::GetInstance()->InitGSData(worldtag,sid);
	}
};

};

#endif
