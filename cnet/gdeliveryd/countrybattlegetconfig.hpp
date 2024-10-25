
#ifndef __GNET_COUNTRYBATTLEGETCONFIG_HPP
#define __GNET_COUNTRYBATTLEGETCONFIG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleGetConfig : public GNET::Protocol
{
	#include "countrybattlegetconfig"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		int start_time = CountryBattleMan::GetInstance()->GetCountryBattleStartTime();
		int end_time = CountryBattleMan::GetInstance()->GetCountryBattleEndTime();
		char is_open = CountryBattleMan::GetInstance()->IsBattleStart();
		int total_bonus = CountryBattleMan::GetInstance()->GetTotalBonus();
		char domain2_datatype = CountryBattleMan::GetInstance()->GetDomain2DataType();
		unsigned int domain2_data_timestamp = get_domain2_data_timestamp();

		CountryBattleGetConfig_Re re(start_time, end_time, total_bonus, is_open, domain2_datatype, domain2_data_timestamp, localsid);
		manager->Send(sid, re);
	}
};

};

#endif
