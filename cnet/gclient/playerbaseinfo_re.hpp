
#ifndef __GNET_PLAYERBASEINFO_RE_HPP
#define __GNET_PLAYERBASEINFO_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "groleforbid"
#include "grolebase"
#include "conv_charset.h"
namespace GNET
{

class PlayerBaseInfo_Re : public GNET::Protocol
{
	#include "playerbaseinfo_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
#ifdef _TESTCODE
		if (retcode == ERR_SUCCESS)
		{
			Octets name_gbk;
			CharsetConverter::conv_charset_u2g(player.name,name_gbk);
			DEBUG_PRINT("\troleid=%d name=%.*s gender=%d race=%d occupation=%d status=%d,custom=%.*s,uiconfig=%.*s\n",player.id,name_gbk.size(),(char*)name_gbk.begin(),player.gender,player.race,player.cls,player.status,player.custom_data.size(),(char*)player.custom_data.begin(),player.config_data.size(),(char*)player.config_data.begin());
		}
		else
		{
			DEBUG_PRINT("Get rolebaseinfo failed.\n");
		}
#endif
	}
};

};

#endif
