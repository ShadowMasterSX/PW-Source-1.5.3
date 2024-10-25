
#ifndef __GNET_FACTIONEXPEL_RE_HPP
#define __GNET_FACTIONEXPEL_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkclient.hpp"
namespace GNET
{

class FactionExpel_Re : public GNET::Protocol
{
	#include "factionexpel_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		int roleid=GLinkClient::GetInstance()->roleid;
		switch (retcode)
		{
			case ERR_SUCCESS:
				if ((unsigned int)roleid==expelroleid)
					printf("�㱻���������\n");
				else 
					printf("�ɹ��������%d\n",expelroleid);
				break;
			case ERR_FC_NO_PRIVILEGE:
				printf("��û��������ڵ�Ȩ��.\n");
				break;	
			case ERR_FC_NOTAMEMBER:
				printf("�������߲��Ǳ����Ա.\n");	
				break;
			case ERR_FC_DBFAILURE:
				printf("���ݿ��д����\n");
				break;	
		}
		FactionChoice(roleid,manager,sid);
	}
};

};

#endif
