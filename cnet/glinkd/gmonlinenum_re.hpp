
#ifndef __GNET_GMONLINENUM_RE_HPP
#define __GNET_GMONLINENUM_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "statusannounce.hpp"
namespace GNET
{

class GMOnlineNum_Re : public GNET::Protocol
{
	#include "gmonlinenum_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		this->local_num=lsm->roleinfomap.size();
		lsm->Send(localsid,this);
	}
};

};

#endif
