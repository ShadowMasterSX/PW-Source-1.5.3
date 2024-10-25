
#ifndef __GNET_TRANSBUYPOINT_RE_HPP
#define __GNET_TRANSBUYPOINT_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "transid"
#include "sellid"
#include "gamedbclient.hpp"
namespace GNET
{

class TransBuyPoint_Re : public GNET::Protocol
{
	#include "transbuypoint_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		Log::formatlog("sellpoint","gdelivery.transbuypoint_re. received. retcode=%d,tid(z:%d,s:%d),sellid(r:%d,s:%d),buyer=%d,price=%d,point=%d,aid=%d\n",retcode,tid.zoneid,tid.serialno,sellid.roleid,sellid.sellid,buyer,price,point,aid);
		GameDBClient::GetInstance()->SendProtocol( this );
	}
};

};

#endif
