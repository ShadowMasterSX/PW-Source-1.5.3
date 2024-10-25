
#ifndef __GNET_SYNMUTADATA_HPP
#define __GNET_SYNMUTADATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class SynMutaData : public GNET::Protocol
{
	#include "synmutadata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
