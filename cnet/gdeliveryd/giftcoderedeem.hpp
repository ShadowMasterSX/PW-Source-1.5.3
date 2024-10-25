
#ifndef __GNET_GIFTCODECASH_HPP
#define __GNET_GIFTCODECASH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "game2au.hpp"
#include "gproviderserver.hpp"
#include "giftcoderedeem_re.hpp"
#include "mapuser.h"

namespace GNET
{

class GiftCodeRedeem : public GNET::Protocol
{
	#include "giftcoderedeem"

	void SendResult(int sid,int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(
				GProviderServer::GetInstance()->FindGameServer(sid),
				GiftCodeRedeem_Re(roleid,cardnumber,1,1,retcode));
	}
	

	void Process(Manager *manager, Manager::Session::ID sid)
	{
	// TODO
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole(roleid);

		if (NULL==pinfo || NULL ==pinfo->user)
		{
			SendResult(sid,GiftCodeRedeem_Re::GCR_INVALID_ROLE);
			Log::log(LOG_ERR, "role%d gift code cash game2au fail", roleid);
			return;
		}

		int64_t lroleid = roleid;
		
		Game2AU proto;
		proto.userid = pinfo->userid;
		proto.qtype = Game2AU::GIFT_CARD_REDEEM;
		proto.info = Marshal::OctetsStream() << lroleid << cardnumber << pinfo->user->ip;
		proto.reserved = 0;

		if(!GAuthClient::GetInstance()->SendProtocol( proto ))
		{
			Log::log(LOG_ERR, "role%d touch cost game2au send fail", roleid);
		}

	}
};

};

#endif
