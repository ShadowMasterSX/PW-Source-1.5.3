
#ifndef __GNET_SYSSENDMAIL3_HPP
#define __GNET_SYSSENDMAIL3_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mailgoodsinventory"

namespace GNET
{

class SysSendMail3 : public GNET::Protocol
{
	#include "syssendmail3"
	
	void MakeAttachObj(GRoleInventory& obj)
	{
		obj.id = attach_goods.goods_id;
		obj.count = attach_goods.count;
		obj.proctype = attach_goods.proctype;
	}
	
	void MakeMail(GMail& mail)
	{
		mail.header.id = 0; // need fill by gamedbd
		mail.header.sender = 32;//iweb sysid
		mail.header.sndr_type = _MST_WEB;
		mail.header.receiver = roleid;
		mail.header.title = mail_title;
		mail.header.send_time = time(NULL);
		mail.header.attribute = (1 << _MA_UNREAD);
		mail.header.sender_name = Octets(sys_name,4);
		
		if( attach_goods.count > 0) {
			mail.header.attribute |= 1 << _MA_ATTACH_OBJ;

			GRoleInventory obj;
			MakeAttachObj(obj);
			mail.attach_obj = obj;
		}

		if(attach_money > 0) {
			mail.header.attribute |= 1 << _MA_ATTACH_MONEY;
			mail.attach_money = attach_money;
		}

		mail.context = mail_context;
	}

	void QueryDB()
	{
		DBSysMail3Arg arg;
		arg.orderid = orderid;
		arg.userid = userid;
		arg.roleid = roleid;
		arg.rolename = rolename;
		arg.goods_flag = attach_goods.goods_flag;
		arg.goods_price =  attach_goods.goods_price;
		arg.goods_price_before_discount =  attach_goods.goods_price_before_discount;
		arg.goods_paytype = attach_goods.paytype;
		MakeMail(arg.mail);

		DBSysMail3* rpc = (DBSysMail3*)Rpc::Call(RPC_DBSYSMAIL3, arg);
		GameDBClient::GetInstance()->SendProtocol(rpc);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("SysSendMail3: receive. orderid=%lld,userid=%d,roleid=%lld,rolename_size=%d,attach_money=%d,goods(id=%d,count=%d,proctype=%d,flag=%d,price=%d,price_before_discount=%d, paytype=%d)\n", 
			orderid, userid, roleid, rolename.size(), attach_money, attach_goods.goods_id, attach_goods.count, attach_goods.proctype, attach_goods.goods_flag, attach_goods.goods_price, 
			attach_goods.goods_price_before_discount, attach_goods.paytype);
		
		//paytype是支付方式，点券支付为1，元宝支付为2，目前只支持点券支付
		if(attach_goods.paytype != 1) return;
		
		QueryDB();
	}
};

};

#endif
