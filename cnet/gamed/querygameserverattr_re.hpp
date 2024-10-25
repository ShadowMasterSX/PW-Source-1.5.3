
#ifndef __GNET_QUERYGAMESERVERATTR_RE_HPP
#define __GNET_QUERYGAMESERVERATTR_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gameattr"
#include "gmattr.h"
namespace GNET
{

class QueryGameServerAttr_Re : public GNET::Protocol
{
	#include "querygameserverattr_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		for ( size_t i=0;i<attr.size();++i )
			OnReportAttr( attr[i].attr,attr[i].value );
	}
};

};

#endif
