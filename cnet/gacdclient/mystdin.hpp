
#ifndef __GNET_MYSTDIN_HPP
#define __GNET_MYSTDIN_HPP

#include "protocol.h"
#include "commander.hpp"

namespace GNET
{

class MyStdin : public GNET::StdInIO
{

	void OnGetLine(std::string line)
	{
		// TODO
		Commander::GetInstance()->ParseCommandLine(line);
	}
};

};

#endif
