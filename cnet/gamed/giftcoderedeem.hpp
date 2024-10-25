
#ifndef __GNET_GIFTCODEREDEEM_HPP
#define __GNET_GIFTCODEREDEEM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GiftCodeRedeem : public GNET::Protocol
{
	#include "giftcoderedeem"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
