
#ifndef __GNET_QPADDCASH_HPP
#define __GNET_QPADDCASH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mapuser.h"
#include "game2au.hpp"
#include "discountman.h"
#include "qpaddcash_re.hpp"

namespace GNET
{

class QPAddCash : public GNET::Protocol
{
	#include "qpaddcash"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// 以下retcode值的含义同AU2Game的retcode值，见au2game.hpp
		
		UserInfo* ui = UserContainer::GetInstance().FindUser(userid);
		if (ui == NULL || ui->linksid != sid)
		{
			return;
		}

		if (ui->activated_merchants.find(merchant_id) == ui->activated_merchants.end())
		{
			// 服务未开通
			QPAddCash_Re proto(ui->localsid, cash, cash_after_discount, merchant_id, 3);
			manager->Send(sid, proto);
			return;
		}

		if (cash % 100 != 0 || cash < 600 || cash > 100000)
		{
			// 告知金额出错
			QPAddCash_Re proto(ui->localsid, cash, cash_after_discount, merchant_id, 5);
			manager->Send(sid, proto);
			return;
		}

		if (!DiscountMan::GetInstance()->CheckDiscount(cash, cash_after_discount, merchant_id))
		{
			// 更新客户端折扣信息
			DiscountMan::GetInstance()->NotifyDiscount(sid, ui->localsid);

			// 告知折扣出错
			QPAddCash_Re proto(ui->localsid, cash, cash_after_discount, merchant_id, 7);
			manager->Send(sid, proto);
			return;
		}

		Game2AU proto;
		proto.userid = userid;
		proto.qtype = Game2AU::PAY_REQUEST;
		proto.info = Marshal::OctetsStream() << cash << cash_after_discount << merchant_id;

		GAuthClient::GetInstance()->SendProtocol( proto );
	}
};

};

#endif
