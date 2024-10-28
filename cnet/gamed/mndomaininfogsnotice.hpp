
#ifndef __GNET_MNDOMAININFOGSNOTICE_HPP
#define __GNET_MNDOMAININFOGSNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mndomaininfo"

namespace GNET
{

class MNDomainInfoGSNotice : public GNET::Protocol
{
	#include "mndomaininfogsnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		for(size_t i = 0; i < domaininfo_list.size(); i++)
		{
			GMSV::SetMnDomain(domaininfo_list[i].domain_id, domaininfo_list[i].domain_type, domaininfo_list[i].owner_unifid, domaininfo_list[i].attacker_unifid, domaininfo_list[i].defender_unifid);
		}
	}
};

};

#endif
