
#ifndef __GNET_TRADESTART_HPP
#define __GNET_TRADESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "trade.h"

#include "gdeliveryserver.hpp"
#include "tradestart_re.hpp"
#include "tradestartrqst.hrp"
#include "announceforbidinfo.hpp"
#include "mapforbid.h"
#include "mapuser.h"
namespace GNET
{

class TradeStart : public GNET::Protocol
{
	#include "tradestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("gdelivery::receive tradestart from linkd,startroleid=%d,partnerroleid=%d\n",
			roleid,partner_roleid);

		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		if(dsm->IsCentralDS())
		{
			DEBUG_PRINT("gdelivery::tradestart:try trade on central delivery, refuse him.roleid=%d,partener=%d\n", roleid, partner_roleid);
			dsm->Send(sid, TradeStart_Re(ERR_TRADE_INVALID_TRADER , _TRADE_ID_INVALID, 0, roleid, localsid));
			return;
		}
		
		//verify users is online
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * ui = UserContainer::GetInstance().FindRoleOnline(roleid);
		if (ui == NULL)
		{
			DEBUG_PRINT("gdelivery::tradestart can not find starter.\n");
		   	return;
		}
		else if (ui->roleid != roleid)
		{
			DEBUG_PRINT("gdelivery::tradestart starter roleid(%d) is wrong.(right is %d).\n",roleid,ui->roleid);
			return;
		}	
		{
			GRoleForbid	forbid;
			if( ForbidTrade::GetInstance().GetForbidTrade( roleid, forbid ) )
			{
				dsm->Send(sid,AnnounceForbidInfo(ui->userid,localsid,forbid));
				DEBUG_PRINT("gdelivery::startroleid(%d) is forbid to trade. time_left is %d\n",
					roleid,forbid.time);
				return;
			}
			//verify partner
			if( ForbidTrade::GetInstance().GetForbidTrade( partner_roleid, forbid ) )			
			{
				dsm->Send(sid,TradeStart_Re(ERR_TRADE_PARTNER_FORBID,_TRADE_ID_INVALID,partner_roleid,
					roleid,localsid));
				return;
			}
		}
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(partner_roleid);
		if (NULL == pinfo )
		{
			DEBUG_PRINT("gdelivery::tradestart can not find partner.roleid=%d,partener=%d\n",roleid,partner_roleid);
			dsm->Send(sid,TradeStart_Re(ERR_TRADE_PARTNER_OFFLINE,_TRADE_ID_INVALID,partner_roleid,roleid,localsid));
			return;
		}
		else if (NULL!=pinfo && (ui->gameid!=pinfo->gameid))
		{
			DEBUG_PRINT("gdelivery::tradestart:trade partner is in different gameserver.roleid=%d,partener=%d\n",
					roleid,partner_roleid);
			dsm->Send(sid,TradeStart_Re(ERR_TRADE_INVALID_TRADER,_TRADE_ID_INVALID,partner_roleid,roleid,localsid));
			return;
		}
		//verify traders
		if (!GNET::Transaction::VerityTraders(roleid,partner_roleid))
		{
			DEBUG_PRINT("gdelivery::tradestart trader is busy.roleid=%d,partner=%d\n",roleid,partner_roleid);
			dsm->Send(sid,TradeStart_Re(ERR_TRADE_BUSY_TRADER,_TRADE_ID_INVALID,partner_roleid,roleid,localsid));
			return;
		}
		TradeStartRqst* trade_rqst=(TradeStartRqst*)Rpc::Call(RPC_TRADESTARTRQST,TradeStartRqstArg(partner_roleid,pinfo->localsid,roleid));
		trade_rqst->starter_localsid=localsid;
		trade_rqst->partner_localsid=pinfo->localsid;
		dsm->Send(pinfo->linksid,trade_rqst);
	}
};

};

#endif
