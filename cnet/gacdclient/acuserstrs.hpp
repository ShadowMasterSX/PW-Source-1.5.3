
#ifndef __GNET_ACUSERSTRS_HPP
#define __GNET_ACUSERSTRS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "ganticheaterclient.hpp"
#include "acconstants.h"

namespace GNET
{

class ACUserStrs : public GNET::Protocol
{
	#include "acuserstrs"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		printf("user modules, processes and windows, area %d, roleid %d\n"
            , ((GAntiCheaterClient*)manager)->GetAreaID(), roleid);
		for(std::map<char, std::vector<std::string> >::const_iterator it = strs.begin();
			it != strs.end(); ++it)
		{
			if( (*it).first == AC_STR_MODULE )
				printf("\nModules");
			else if( (*it).first == AC_STR_PROCESS )
				printf("\nProcesses");
			else if( (*it).first == AC_STR_WINDOW )
				printf("\nWindows");
			const std::vector<std::string> &tstrs = (*it).second;
			printf(", count %d\n", tstrs.size());
			for(std::vector<std::string>::const_iterator it2 = tstrs.begin();
				it2 != tstrs.end(); ++it2)
				printf("[%s] ", (*it2).c_str());
			
		}
	}
};

};

#endif
