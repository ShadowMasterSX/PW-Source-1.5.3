
#ifndef __GNET_ACUSERONLINE_HPP
#define __GNET_ACUSERONLINE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "ganticheaterclient.hpp"

namespace GNET
{

class ACUserOnline : public GNET::Protocol
{
	#include "acuseronline"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		printf("user online, area %d\n"
            , ((GAntiCheaterClient*)manager)->GetAreaID());
        for(std::map<int, std::vector<int> >::iterator it = users.begin();
            it != users.end(); ++it)
        {
            printf( "[link id : %d]\n", (*it).first);
			std::vector<int> &vecs = (*it).second;
			for(std::vector<int>::iterator it2 = vecs.begin(); it2 != vecs.end(); ++it2)
			{
				printf("%d\t", (*it2));
			}
            printf( "\n");
        }
	}
};

};

#endif
