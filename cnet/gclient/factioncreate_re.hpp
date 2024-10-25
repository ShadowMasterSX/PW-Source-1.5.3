
#ifndef __GNET_FACTIONCREATE_RE_HPP
#define __GNET_FACTIONCREATE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkclient.hpp"
#include "factionchoice.h"
namespace GNET
{

class FactionCreate_Re : public GNET::Protocol
{
	#include "factioncreate_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		switch(retcode)
		{
		case ERR_SUCCESS:
			printf("���%d�������ɣ�%.*s�ɹ�.\n",roleid,faction_name.size(),(char*)faction_name.begin());
			break;
		case ERR_FC_CREATE_ALREADY:
			printf("���%d�Ѿ���ĳ�����ɵĳ�Ա�������ٴ�������.\n",roleid);
			break;
		case ERR_FC_CREATE_DUP:
			printf("���������ظ�.\n");
			break;	
		case ERR_FC_DBFAILURE:
			printf("���ݿ��д����\n");
			break;	
		case ERR_FC_NO_PRIVILEGE:
			printf("û��Ȩ�ޡ������Ѿ���ĳ�����ɵĳ�Ա\n");
			break;	
		}	
		int roleid=GLinkClient::GetInstance()->roleid;
		FactionChoice(roleid,manager,sid);
	}
};

};

#endif
