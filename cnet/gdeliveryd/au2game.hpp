
#ifndef __GNET_AU2GAME_HPP
#define __GNET_AU2GAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "mapuser.h"
#include "qpaddcash_re.hpp"
#include "qpgetactivatedservices_re.hpp"
#include "mint"
#include "touchpointcost_re.hpp"
#include "touchpointquery_re.hpp"
#include "auaddupmoneyquery_re.hpp"

namespace GNET
{

class AU2Game : public GNET::Protocol
{
	#include "au2game"

	// retcode各值含义：
	// 0. 订单已提交，短信校验码已免费发送到您手机（Pay返回的)；
	// 1. 订单提交失败（Pay返回的）；
	// 2. 账号不存在；
	// 3. 账号尚未开通此支付功能；
	// 4. 网络通信错误；
	// 5. 支付金额不符合要求；
	// 6. 计费区不存在；
	// 7. 折扣信息错误（Pay返回的）；
	// 8. 其他错误。

	enum
	{
		GIFT_CARD_REDEEM = 3,
		PAY_RESPONSE = 4,
		ACTIVATED_MERCHANTS = 6,
		QUERY_TOUCH_POINT = 8,
		COST_TOUCH_POINT = 9,
		QUERY_ADDUP_MONEY = 12,
	};

	void SendResult(int gsid,Protocol& proto)
	{
		if(!GProviderServer::GetInstance()->DispatchProtocol(gsid,proto))
		{
			Log::log(LOG_ERR, "au2game, sendback to gs fail qtype=%d retcode=%d user%d gsid=%d",qtype,retcode,userid,gsid);
		}
	}


	void Process(Manager *manager, Manager::Session::ID sid)
	{
		UserInfo* ui = UserContainer::GetInstance().FindUser(userid);
		if (ui == NULL)
		{
			Log::log(LOG_ERR, "au2game, qtype=%d, failed to find user%d",qtype, userid);
			return;
		}

		switch(qtype)
		{
			case(GIFT_CARD_REDEEM):
			{
					int64_t lroleid = 0;					
					Octets cardnumber;
					int type = 0, parenttype = 0;
					int roleid = 0;

					try
					{
						Marshal::OctetsStream(info) >> lroleid >> cardnumber >> type >> parenttype;
					}
					catch(Marshal::Exception& e)
					{
						retcode = GiftCodeRedeem_Re::GCR_UNMARSHAL_FAIL;
						Log::log(LOG_ERR, "au2game, qtype=GIFT_CARD_REDEEM, role%d failed to unmarshal info",ui->roleid);
					}

					roleid = (int)lroleid;

					if(roleid && roleid == ui->roleid)
					{
						GiftCodeRedeem_Re proto(roleid,cardnumber,type,parenttype,retcode);
						SendResult(ui->gameid,proto);
					}

					Log::log(LOG_NOTICE, "Gift Card Redeem , ret%d type %d parenttype %d roleid %d gid%d",
								retcode, type, parenttype, ui->roleid, ui->gameid );// 

			}
			break;
			case(PAY_RESPONSE):
			{
				int cash;
				int cash_after_discount;
				int merchant_id;
	
				try
				{
					Marshal::OctetsStream(info) >> cash >> cash_after_discount >> merchant_id;
				}
				catch(Marshal::Exception& e)
				{
					Log::log(LOG_ERR, "au2game, qtype=%d, failed to unmarshal info",qtype);
					break;
				}
	
				if (retcode == 7)
				{
					// 尝试更正客户端的折扣信息
					DiscountMan::GetInstance()->NotifyDiscount(ui->linksid, ui->localsid);
				}
	
				QPAddCash_Re proto(ui->localsid, cash, cash_after_discount, merchant_id, retcode);
				GDeliveryServer::GetInstance()->Send(ui->linksid, proto);
				break;
			}
			case(ACTIVATED_MERCHANTS):
			{
				if (ui->status != _STATUS_ONGAME)
				{
					return;
				}

				// 同时更新一下客户端的折扣信息
				DiscountMan::GetInstance()->NotifyDiscount(ui->linksid, ui->localsid);
	
				MIntVector merchants;
				try
				{
					Marshal::OctetsStream(info) >> merchants;
				}
				catch(Marshal::Exception& e)
				{
					Log::log(LOG_ERR, "au2game, qtype=%d, failed to unmarshal info to MintVector", qtype);
					break;
				}

				ui->activated_merchants.clear();
				std::vector<int> merchants_copy;
				for (unsigned int i = 0; i < merchants.size(); i++)
				{
					merchants_copy.push_back(merchants[i].id);
					ui->activated_merchants.insert(merchants[i].id);
				}
				QPGetActivatedServices_Re proto(ui->localsid, merchants_copy, retcode);
				GDeliveryServer::GetInstance()->Send(ui->linksid, proto);
				break;
			}
			case QUERY_TOUCH_POINT:
			{
				int64_t income = 0,remain = 0;
				
				if(retcode == 0)
				{
					try
					{
						Marshal::OctetsStream(info) >> income >> remain;
					}
					catch(Marshal::Exception& e)
					{
						income = 0;
						remain = 0;
						Log::log(LOG_ERR, "au2game, qtype=QUERY_TOUCH_POINT, role%d failed to unmarshal info",ui->roleid);
					}

					TouchPointQuery_Re proto(ui->roleid,income,remain);
					SendResult(ui->gameid,proto);
				}
				else
				{
				
					Log::log(LOG_ERR, "au2game, qtype=QUERY_TOUCH_POINT, role%d retcode%d",ui->roleid, retcode);
				}
			}
			break;
			case COST_TOUCH_POINT:
				{
					int64_t orderid = 0,income = 0, remain = 0;
					int cost = 0, roleid = 0;
					Octets context;
					
					try
					{
						Marshal::OctetsStream(info) >> orderid >> cost >> context >> income >> remain;
						Marshal::OctetsStream(context) >> roleid;
					}
					catch(Marshal::Exception& e)
					{
						roleid = 0;
						retcode = TouchPointCost_Re::TPC_OTHER;
						Log::log(LOG_ERR, "au2game, qtype=COST_TOUCH_POINT, role%d failed to unmarshal info",ui->roleid);
					}

					if(roleid && roleid == ui->roleid)
					{
						TouchPointCost_Re proto(roleid,orderid,cost,income,remain,retcode);
						SendResult(ui->gameid,proto);
					}

					Log::log(LOG_NOTICE, "Touch Cost, SN%lld ret%d cost%d income %lld remain %lld roleid %d gid%d",
								orderid, retcode, cost, income, remain, ui->roleid, ui->gameid );// 唯一的touch cost 交易log

				}
				break;
			case QUERY_ADDUP_MONEY:
				{
					int64_t addupmoney = 0;
				
					if(retcode == 0)
					{
						try
						{
							Marshal::OctetsStream(info) >> addupmoney;
						}
						catch(Marshal::Exception& e)
						{
							Log::log(LOG_ERR, "au2game, qtype=QUERY_ADDUP_MONEY,role%d failed to unmarshal info",ui->roleid);
						}

						AuAddupMoneyQuery_Re proto(ui->roleid,addupmoney);
						SendResult(ui->gameid,proto);
				
					}
				}
				break;
		}
	}
};

};

#endif
