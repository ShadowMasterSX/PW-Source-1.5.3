#include "types.h"
#include "obj_interface.h"
#include "mailsyslib.h"

#include "libcommon.h"

#include "gmailsyncdata"
#include "gproviderclient.hpp"
#include "deletemail.hpp"
#include "getmailattachobj.hpp"
#include "getmail.hpp"
#include "getmaillist.hpp"
#include "playersendmail.hpp"
#include "playersendmassmail.hpp"
#include "preservemail.hpp"
#include "../gdbclient/db_if.h"
#include <iconv.h>

#define MAIL_FEE             500
#define MASS_MAIL_FEE             500
#define MAX_MASS_CONTEXT_LEN 	400
#define MAX_MASS_TITLE_LEN 	40
#define GFACTION_SERVER_ID 101
#define GDELIVERY_SERVER_ID  0
#define CASE_PROTO_HANDLE(_proto_name_)\
	case _proto_name_::PROTOCOL_TYPE:\
	{\
		_proto_name_ proto;\
		proto.unmarshal( os );\
		if ( proto.GetType()!=_proto_name_::PROTOCOL_TYPE || !proto.SizePolicy(os.size()) )\
			return false;\
		return Handle_##_proto_name_( proto,obj_if );\
	}

namespace GNET
{

	bool Handle_DeleteMail( DeleteMail& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
	}
	bool Handle_GetMailAttachObj( GetMailAttachObj& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) 
			return false;

		GMailSyncData data;
		if ( !GetSyncData(data,obj_if) ) 
			return false;

		proto.localsid          = obj_if.GetLinkSID();

		Marshal::OctetsStream os;
		os << data;
		proto.syncdata = os;
		if(obj_if.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT)==0)
		{
			if ( GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto ) )
				return true;
			obj_if.TradeUnLockPlayer();
		}
		return false;
	}

	bool Handle_GetMail( GetMail& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
	}
	bool Handle_GetMailList( GetMailList& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
	}		
	bool Handle_PlayerSendMail( PlayerSendMail& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;

		GMailSyncData data;
		if ( !GetSyncData(data,obj_if) ) 
			return false;

		if ( data.inventory.money < proto.attach_money+MAIL_FEE || data.inventory.money<0)
			 return false;
		if ( proto.attach_obj_num!=0 && !obj_if.CheckItem(proto.attach_obj_pos,proto.attach_obj_id,
				proto.attach_obj_num) ) 
			return false;

		proto.localsid = obj_if.GetLinkSID();
		Marshal::OctetsStream os;
		os << data;
		proto.syncdata = os;
		proto.localsid = obj_if.GetLinkSID();
		if(obj_if.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT)==0)
		{
			if (GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto ))
				return true;
			obj_if.TradeUnLockPlayer();
		}
		return false;
	}	
	bool Handle_PreserveMail( PreserveMail& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
	}	
	bool Handle_PlayerSendMassMail( PlayerSendMassMail& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		if ( proto.mass_type >= MMT_TYPECOUNT ) return false;
		if ( proto.context.size() > MAX_MASS_CONTEXT_LEN ) return false;
		if ( proto.title.size() > MAX_MASS_TITLE_LEN ) return false;

		switch(proto.mass_type)
		{
			case MMT_FACTION:
				{
					if(obj_if.GetMafiaRank() > _R_BODYGUARD)
						return false;
					
					proto.cost_money = MASS_MAIL_FEE * proto.receiver_list.size();
					proto.cost_obj_num = 0;
					proto.cost_obj_id = proto.cost_obj_pos = -1;
					proto.syncdata.clear();
					proto.sender_name.clear();
				}
				break;
			default:
				return false;
		}		

		GMailSyncData data;
		if ( !GetSyncData(data,obj_if) ) 
			return false;

		if ( data.inventory.money < proto.cost_money || data.inventory.money<0)
			return false;

		if ( proto.cost_obj_num && !obj_if.CheckItem(proto.cost_obj_pos, proto.cost_obj_id, proto.cost_obj_num))
			return false;	

		proto.localsid = obj_if.GetLinkSID();
		Marshal::OctetsStream os;
		os << data;
		proto.syncdata = os;
		proto.localsid = obj_if.GetLinkSID();
		
		if(obj_if.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT)==0)
		{
			switch(proto.mass_type)
			{
				case MMT_FACTION:
					{
						if (GProviderClient::GetInstance()->DispatchProtocol( GFACTION_SERVER_ID,proto ))
							return true;
					}
					break;
			}
			
			obj_if.TradeUnLockPlayer();
		}
		return false;
	}	

	bool ForwardMailSysOP( unsigned int type,const void* pParams,size_t param_len,object_interface obj_if )
	{
		try {
			Marshal::OctetsStream os( Octets(pParams,param_len) );
			switch (type)
			{
				CASE_PROTO_HANDLE(DeleteMail)
				CASE_PROTO_HANDLE(GetMailAttachObj)
				CASE_PROTO_HANDLE(GetMail)
				CASE_PROTO_HANDLE(GetMailList)
				CASE_PROTO_HANDLE(PlayerSendMail)
				CASE_PROTO_HANDLE(PreserveMail)
				CASE_PROTO_HANDLE(PlayerSendMassMail)
				default:
					return false;	
			}
		}
		catch ( Marshal::Exception )
		{
			return false;
		}
	}

	void TestMassMail(unsigned char type,object_interface obj_if)
	{
		PlayerSendMassMail proto;
		proto.roleid = obj_if.GetSelfID().id ;
		proto.mass_type = type ;
		char msg[24] = {"»∫∑¢≤‚ ‘\n"};
		
		iconv_t cd = iconv_open("UCS2", "GBK");
		if(cd == (iconv_t)-1) return;
		size_t isize = strlen(msg);
		char * ibuf = (char *)msg;
		size_t osize = isize * 2;
		char * obuf = (char *)malloc(osize);
		char * tmpbuf = obuf;
		size_t tmpsize = osize;
	
		if(iconv(cd, &ibuf, &isize, &tmpbuf, &tmpsize) == (size_t)(-1))
		{
			iconv_close(cd);
			free(obuf);
			return;
		}

		Octets(obuf,osize-tmpsize).swap(proto.title);
		proto.context = proto.title;
		
		Handle_PlayerSendMassMail(proto,obj_if);
		
		iconv_close(cd);
		free(obuf);
	}
}
