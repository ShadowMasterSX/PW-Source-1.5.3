
#ifndef __GNET_FACEMODIFY_RE_HPP
#define __GNET_FACEMODIFY_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class FaceModify_Re : public GNET::Protocol
{
	#include "facemodify_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
