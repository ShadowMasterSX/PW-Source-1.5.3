
#ifndef __GNET_ACUSERLOGINOUTINFO_HPP
#define __GNET_ACUSERLOGINOUTINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "ganticheaterclient.hpp"

namespace GNET
{

class ACUserLoginoutInfo : public GNET::Protocol
{
	#include "acuserloginoutinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		printf("user login and logout, area %d, roleid %d\n"
            , ((GAntiCheaterClient*)manager)->GetAreaID(), roleid);
        for(std::deque<std::pair<bool, time_t> >::iterator it = logs.begin();
            it != logs.end(); ++it)
        {
            printf( (*it).first ? "==>login  " : "<==logout ");
            printf( "at %s \n", ctime(&((*it).second)));
        }
	}
};

};

#endif
