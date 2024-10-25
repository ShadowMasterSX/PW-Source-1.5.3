
#ifndef __GNET_UNIQUEDATAMODIFYNOTICE_HPP
#define __GNET_UNIQUEDATAMODIFYNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void OnUniqueDataModify(int worldtag, int key, int type, const void* val, size_t sz,const void* oldval, size_t osz, int retcode, int version);

namespace GNET
{

class UniqueDataModifyNotice : public GNET::Protocol
{
	#include "uniquedatamodifynotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		OnUniqueDataModify(worldtag, key, vtype, value.begin(), value.size(), oldvalue.begin(), oldvalue.size(), retcode, version);
	}
};

};

#endif
