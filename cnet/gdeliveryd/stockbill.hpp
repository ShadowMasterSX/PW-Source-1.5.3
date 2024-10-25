
#ifndef __GNET_STOCKBILL_HPP
#define __GNET_STOCKBILL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "stockbill_re.hpp"
#include "stockexchange.h"

namespace GNET
{

class StockBill : public GNET::Protocol
{
        #include "stockbill"

        void Process(Manager *manager, Manager::Session::ID sid)
        {
                Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
                PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
                if ( pinfo )
                {
                        StockBill_Re res;
                        res.localsid = pinfo->localsid;
                        if(StockExchange::Instance()->TradeLog(pinfo->userid,res.orders,res.logs))
                                GDeliveryServer::GetInstance()->Send(pinfo->linksid,res);
                }
        }
};

};

#endif
