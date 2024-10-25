
#ifndef __GNET_FACTIONCREATE_HPP
#define __GNET_FACTIONCREATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "operwrapper.h"
namespace GNET
{

class FactionCreate : public GNET::Protocol
{
	#include "factioncreate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		unsigned int tid=OperWrapper::CreateWrapper(roleid,_O_FACTION_CREATE);
		OperWrapper::href_t op=OperWrapper::href_t( OperWrapper::FindWrapper(tid) );
		if ( op )
		{
			op->SetParams(Marshal::OctetsStream()<<faction_name);
			op->Execute();
		}		
	}
};

};

#endif
