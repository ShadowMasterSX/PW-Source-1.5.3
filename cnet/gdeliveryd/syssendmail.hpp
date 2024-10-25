
#ifndef __GNET_SYSSENDMAIL_HPP
#define __GNET_SYSSENDMAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "postoffice.h"
#include "dbsendmail.hrp"
#include "syssendmail_re.hpp"

namespace GNET
{
static char sys_name[] = {'G',0,'M',0};
class SysSendMail : public GNET::Protocol
{
	#include "syssendmail"
	void MakeMail(GMail& mail)
	{
		mail.header.id        = 0; // need fill by gamedbd
		mail.header.sender    = sysid;
		mail.header.sndr_type = _MST_WEB;
		mail.header.receiver  = receiver;
		mail.header.title     = title;
		mail.header.send_time = time(NULL);
		mail.header.attribute = (1<<_MA_UNREAD);
		mail.header.sender_name = Octets(sys_name,4);
		if ( attach_obj.count )
		{
			mail.header.attribute |= 1<<_MA_ATTACH_OBJ;
			mail.attach_obj=attach_obj;
		}
		if ( attach_money )
		{
			mail.header.attribute |= 1<<_MA_ATTACH_MONEY;
			mail.attach_money=attach_money;
		}
		mail.context = context;
	}
	bool QueryDB( unsigned int linksid )
	{
		DBSendMailArg arg;
		MakeMail( arg.mail );
		DBSendMail* rpc=(DBSendMail*) Rpc::Call( RPC_DBSENDMAIL, arg);
		rpc->save_linksid=linksid;
		rpc->save_localsid=tid;
		rpc->save_gsid=0;
		rpc->send_web_reason = DBSendMail::REASON_SEND_WEB_SYSSENDMAIL;

		return GameDBClient::GetInstance()->SendProtocol( rpc );
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("sys_sendmail: receive. sysid=%d(tid=%d),receiver=%d,attach_obj(id:%d,pos:%d,num:%d),attach_money=%d\n", 
			sysid,tid,receiver,attach_obj.id,attach_obj.pos,attach_obj.count,attach_money);
		if ( !QueryDB(sid) && sid!=0 )
			manager->Send( sid,SysSendMail_Re(ERR_COMMUNICATION,tid) );
	}
};

};

#endif
