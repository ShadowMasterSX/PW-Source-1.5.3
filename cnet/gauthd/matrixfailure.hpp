
#ifndef __GNET_MATRIXFAILURE_HPP
#define __GNET_MATRIXFAILURE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MatrixFailure : public GNET::Protocol
{
	#include "matrixfailure"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
