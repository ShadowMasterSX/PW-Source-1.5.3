
#ifndef __GNET_GAMEDATASEND_HPP
#define __GNET_GAMEDATASEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "vclient_if.h"
#include "glinkclient.hpp"

namespace GNET
{

class GamedataSend : public GNET::Protocol
{
	#include "gamedatasend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
/*		
		static size_t count=0;
		if ( (++count % 10) == 0 )
			printf("Receive %d GamedataSend protocol.\n",count);
*/			
#ifdef _TESTCODE //liping
		VCLIENT::RecvS2CGameData(((GLinkClient*)manager)->roleid, data.begin(), data.size());	
#endif
	}
};

};

#endif
