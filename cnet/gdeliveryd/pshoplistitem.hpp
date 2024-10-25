#ifndef __GNET_PSHOPLISTITEM_HPP
#define __GNET_PSHOPLISTITEM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "pshopmarket.h"
#include "gdeliveryserver.hpp"
#include "pshoplistitem_re.hpp"

namespace GNET
{

class PShopListItem : public GNET::Protocol
{
	#include "pshoplistitem"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		if(itemid <= 0 || (listtype != 0 && listtype != 1)) return;
		
		/*
		 * 协议中page_num为客户端希望请求的页面号
		 * 下面定义的pagenum为服务器实际返回的页面
		 * 在特殊情况下,返回页面和实际请求页面不一致
		 * 例如请求页面超出已存在页面范围,则返回最后一页
		 */
		int pagenum = 0;
		PShopItemEntryVector list;
		PShopMarket::find_param_t param(listtype, itemid, page_num);
		PShopMarket::GetInstance().ListItems(param, list, pagenum);
		GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopListItem_Re(pinfo->localsid, list, listtype, pagenum));
	}
};

};

#endif
