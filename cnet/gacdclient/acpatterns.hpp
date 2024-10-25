
#ifndef __GNET_ACPATTERNS_HPP
#define __GNET_ACPATTERNS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "ganticheaterclient.hpp"

namespace GNET
{

class ACPatterns : public GNET::Protocol
{
	#include "acpatterns"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		 printf("total patterns, area %d\n"
            , ((GAntiCheaterClient*)manager)->GetAreaID());
		if( roleid == -1 )
		{
			printf("pattern type counts %d\n", patterns.size());
			for(std::map<int,std::pair<size_t,size_t> >::const_iterator it = patterns.begin();
				it != patterns.end(); ++it)
				printf("pattern %08xh, user count %d, total count %d\n", 
					(*it).first, (*it).second.first, (*it).second.second);
		}
		else
		{
			printf("roleid %d, patttern type counts %d\n", roleid, patterns.size());
			std::map<std::pair<std::pair<int,size_t>,int>,size_t> tmap;
			for(std::map<int,std::pair<size_t,size_t> >::const_iterator it = patterns.begin();
				it != patterns.end(); ++it)
			{
				//printf("pattern %d, count %d,\t", 
				//	(*it).first, (*it).second.second);
				std::pair<int,std::pair<size_t,int> > p = 
					GAntiCheaterClient::GetInstance()->GetBrief((*it).first);
				tmap[std::make_pair(std::make_pair(p.first,p.second.first),p.second.second)] = (*it).second.second;
				//printf("caller %d, size %d, realpattern %d\n",
				//	p.first, p.second.first, p.second.second);
			}
			for(std::map<std::pair<std::pair<int,size_t>,int>,size_t>::const_iterator it = tmap.begin();
				it != tmap.end(); ++it)
				printf("caller %08xh size %d ==> pattern %08xh count %d\n", (*it).first.first.first, (*it).first.first.second,
					(*it).first.second, (*it).second);
		}
	}
};

};

#endif
