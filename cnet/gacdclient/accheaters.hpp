
#ifndef __GNET_ACCHEATERS_HPP
#define __GNET_ACCHEATERS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "ganticheaterclient.hpp"
#include "cheatersdb.h"

namespace GNET
{

class ACCheaters : public GNET::Protocol
{
	#include "accheaters"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		printf("get cheaters, area %d\n"
            , ((GAntiCheaterClient*)manager)->GetAreaID());
		/*
		std::map<int, std::pair<std::string, std::vector<std::pair<int, std::pair<int, time_t> > > > > _cheaters;
			_cheaters[2].second.push_back(std::make_pair(1,std::make_pair(2,time(0))));
			_cheaters[2].second.push_back(std::make_pair(1,std::make_pair(3,time(0))));
			_cheaters[2].second.push_back(std::make_pair(1,std::make_pair(4,time(0))));
			_cheaters[2].second.push_back(std::make_pair(1,std::make_pair(5,time(0))));
		*/
		CheatersDB::SaveCheaters(/*_*/cheaters, ((GAntiCheaterClient*)manager)->GetAreaID());
	}
};

};

#endif
