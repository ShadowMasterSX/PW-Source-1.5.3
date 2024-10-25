
#ifndef __GNET_STOCKTRANSACTION_HPP
#define __GNET_STOCKTRANSACTION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "localmacro.h"
#include "stocktransaction_re.hpp"

namespace GNET
{

class StockTransaction : public GNET::Protocol
{
        #include "stocktransaction"

        void Process(Manager *manager, Manager::Session::ID sid)
        {
                DEBUG_PRINT("StockTransaction, roleid=%d,withdraw=%d,cash=%d,money=%d,cash_total=%d,cash_used=%d,pocket_money=%d",
                        roleid,withdraw,cash,money,syncdata.cash_total,syncdata.cash_used,syncdata.inventory.money);
                PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
                if (!pinfo )
			return;
                int ret = StockExchange::Instance()->PreTransaction(pinfo->userid, withdraw, cash, money,
                        syncdata.cash_total-syncdata.cash_used, syncdata.inventory.money );
                if(ret)
                {
			manager->Send(sid, GMailEndSync(0,ret,roleid,syncdata));
                        GDeliveryServer::GetInstance()->Send(pinfo->linksid, StockTransaction_Re(ret, 0, 0, pinfo->localsid));
                        return;
                }

                DBStockTransactionArg arg(pinfo->userid, roleid, withdraw, cash, money);
                // Utility::SwapSyncData(arg.syncdata,syncdata);
                arg.syncdata = syncdata;
                DBStockTransaction* rpc = (DBStockTransaction*)Rpc::Call( RPC_DBSTOCKTRANSACTION, arg);
                rpc->gamesid = sid;
                GameDBClient::GetInstance()->SendProtocol(rpc);

        }
};

};

#endif
