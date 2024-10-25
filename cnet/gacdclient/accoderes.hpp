
#ifndef __GNET_ACCODERES_HPP
#define __GNET_ACCODERES_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "ganticheaterclient.hpp"

namespace GNET
{

class ACCodeRes : public GNET::Protocol
{
	#include "accoderes"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		printf("res of code send, area %d\n"
            , ((GAntiCheaterClient*)manager)->GetAreaID());
		if( rescode == AC_RES_BADREQUEST )
		{
			printf("server said bad request!\n");
			return;
		}

		//printf("res size %d\n", res.size());
	    typedef std::vector<std::pair<short,int> > Res;
    	typedef std::vector<int> VI;
	    typedef std::map<char,VI> ErrSets;
    	typedef std::map<Res,VI> OkSets;
		typedef std::map<int, std::pair<char, Res> > ResMap;

		ErrSets errsets;
		OkSets oksets;

		for(ResMap::const_iterator it = coderes.begin(); it != coderes.end(); ++it)
		{
			int uid = (*it).first;
			char tstate = (*it).second.first;
			//printf("uid %d, state %d\n", uid, tstate);
			if( tstate != AC_CODE_RES_OK )
			{
				errsets[tstate].push_back(uid);
				continue;
			}
			oksets[(*it).second.second].push_back(uid);
		}

 		for(OkSets::const_iterator it = oksets.begin(); it != oksets.end(); ++it)
        {
            const Res &tres = (*it).first;
            printf("-------------------------\n");
            for(Res::const_iterator it2 = tres.begin(); it2 != tres.end(); ++it2)
            {
                printf("-- funcid %d, res %d --\n", (*it2).first, (*it2).second);
            }
            printf("-------------------------\n");
            const VI &vi = (*it).second;
            printf("user count of this res %d\n", vi.size());
            for(VI::const_iterator it2 = vi.begin(); it2 != vi.end(); ++it2)
            {
                printf("%d ", (*it2));
            }
            printf("\n========\n");
        }
        for(ErrSets::const_iterator it =errsets.begin(); it != errsets.end(); ++it)
        {
            
            if( (*it).first == AC_CODE_RES_OFFLINE )
                printf(" offline\t");
            else if( (*it).first == AC_CODE_RES_ERR )
                printf(" err data\t");
            else if( (*it).first == AC_CODE_RES_INIT )
                printf(" no response\t");
            const VI &vi = (*it).second;
            printf(": %d\n", vi.size());
            for(VI::const_iterator it2 = vi.begin(); it2 != vi.end(); ++it2)
            {
                printf("%d ", (*it2));
            }
            printf("\n=======\n");
        }


	}
};

};

#endif
