
#ifndef __GNET_SYSRECOVEREDOBJMAIL_HPP
#define __GNET_SYSRECOVEREDOBJMAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class SysRecoveredObjMail : public GNET::Protocol
{
	#include "sysrecoveredobjmail"

	void Convert(Octets& dst, const Octets& src)
	{
		const char* p = (const char*)src.begin();
		
		for(size_t i = 0; i < src.size(); i += 2) {
			int tmp = 0;
			sscanf(&p[i], "%02x", &tmp);
			unsigned char c = tmp;
			dst.insert(dst.end(), &c, sizeof(unsigned char));
		}
	}
	
	void Decrpyt(Octets& x)
	{
		Octets key(arc4_key_buf, 128);
		Security::Type sec_type = ARCFOURSECURITY;
		Security *sec = Security::Create(sec_type);
		sec->SetParameter(key);
		sec->Update(x);
		sec->Destroy();
	}
	
	bool MakeMail(GMail& mail)
	{
		bool ret = false;
		
		mail.header.id = 0; // need fill by gamedbd
		const int sysid = 32; //iweb sysid
		mail.header.sender = sysid;
		mail.header.sndr_type = _MST_WEB;
		mail.header.receiver = receiver;
		mail.header.title = title;
		mail.header.send_time = time(NULL);
		mail.header.attribute = (1 << _MA_UNREAD);
		
		if(obj.size() % 2 != 0) return ret;
		Octets result;
		Convert(result, obj);
		Decrpyt(result);

		Octets new_checksum = MD5Hash::Digest(result);
		Octets checksum_convert;
		Convert(checksum_convert, checksum);
		if(checksum_convert != new_checksum) return ret;	
		
		GRoleInventory inventory;
		try {       
			Marshal::OctetsStream(result) >> inventory;
			ret = true;
		} catch ( ... ) {
			ret = false;
		} 

		if ( inventory.count ) {
			mail.header.attribute |= 1 << _MA_ATTACH_OBJ;
			mail.attach_obj = inventory;
		}
		
		mail.context = context;
		return ret;
	}
	
	int QueryDB( unsigned int linksid )
	{
		DBSendMailArg arg;
		if( !MakeMail( arg.mail ) ) return ERR_VERIFYFAILED;
		
		DBSendMail* rpc=(DBSendMail*) Rpc::Call( RPC_DBSENDMAIL, arg);
		rpc->save_linksid = linksid;
		rpc->save_localsid = tid;
		rpc->save_gsid = 0;
		rpc->send_web_reason = DBSendMail::REASON_SEND_WEB_SYSRECOVEREDOBJ;
		
		GameDBClient::GetInstance()->SendProtocol( rpc ); 
		return ERR_SUCCESS;
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("sys_recoveredobjmail: receive. tid=%d, receiver=%d\n", tid, receiver);
		
		int ret = QueryDB(sid);
		
		if(ret != ERR_SUCCESS && sid != 0) {
			manager->Send( sid, SysRecoveredObjMail_Re(ret, tid) );
		}
	}
};

};

#endif
