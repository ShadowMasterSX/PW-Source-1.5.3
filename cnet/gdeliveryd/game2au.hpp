
#ifndef __GNET_GAME2AU_HPP
#define __GNET_GAME2AU_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class Game2AU : public GNET::Protocol
{
	#include "game2au"

	enum
	{
		GIFT_CARD_REDEEM = 3,
		PAY_REQUEST = 4,
		GET_ACTIVATED_MERCHANTS = 6,
		QUERY_TOUCH_POINT = 8,
		COST_TOUCH_POINT = 9,
		QUERY_ADDUP_MONEY = 12,
        COLLECT_CLIENT_MACHINE_INFO = 14,
    };

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
