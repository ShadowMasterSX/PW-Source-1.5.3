
#ifndef __GNET_SSOGETTICKET_HPP
#define __GNET_SSOGETTICKET_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "ssouser"
#include "ssoextrainfo1"
#include "../include/localmacro.h"

bool query_player_info(int roleid, char * name, size_t& name_len, int& level, int& sec_level, int& reputation, int& create_time, int& factionid,
						int itemid1, int& itemcount1, int itemid2, int& itemcount2, int itemid3, int& itemcount3,int& reincarn_times, int& realm_level );

namespace GNET
{

class SSOGetTicket : public GNET::Protocol
{
	#include "ssogetticket"

	enum
	{
		EXTRA_INFO1_CUR_VERSION = 2,
	};

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		Marshal::OctetsStream info_os(info);
		int need_extra_info = SSO_GET_TICKET_EXTRA_INFO_NONE;
		try{ info_os >> need_extra_info; }catch(Marshal::Exception){ return; }
		
		switch(need_extra_info)
		{
			case SSO_GET_TICKET_EXTRA_INFO1:
			{
				int roleid = 0;
				try{ info_os >> roleid; }catch(Marshal::Exception){ return; }

				SSOExtraInfo1 extra;
				extra.roleid = roleid;
				extra.item.push_back(std::pair<int, int>(45591,0));
				extra.item.push_back(std::pair<int, int>(0,0));
				extra.item.push_back(std::pair<int, int>(0,0));
				char name[40] = {0};
				size_t name_len = sizeof(name);

				bool rst = query_player_info(extra.roleid,
						name, name_len,
						extra.level,
						extra.sec_level,
						extra.reputation,
						extra.create_time,
						extra.factionid,
						extra.item[0].first,extra.item[0].second,
						extra.item[1].first,extra.item[1].second,
						extra.item[2].first,extra.item[2].second,
						extra.reincarn_times,
						extra.realm_level);
				if(rst) 
				{
					extra.rolename.insert(extra.rolename.end(),name,name_len);
					this->info = Marshal::OctetsStream() << need_extra_info << (int)EXTRA_INFO1_CUR_VERSION << extra;
					GProviderClient::DispatchProtocol(0,this);
				}
			}
			break;
				
			default:
				break;
		}		
	}
};

};

#endif
