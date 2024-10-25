
#ifndef __GNET_DISCOUNTANNOUNCE_HPP
#define __GNET_DISCOUNTANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "merchantdiscount"
#include "discountman.h"

namespace GNET
{

class DiscountAnnounce : public GNET::Protocol
{
	#include "discountannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DiscountMan::GetInstance()->UpdateDiscount(discount);
	}
};

};

#endif
