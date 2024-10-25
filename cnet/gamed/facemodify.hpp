
#ifndef __GNET_FACEMODIFY_HPP
#define __GNET_FACEMODIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class FaceModify : public GNET::Protocol
{
	#include "facemodify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
