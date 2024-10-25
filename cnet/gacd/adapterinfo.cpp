#include "adapterinfo.h"
#include "stringhelper.hpp"

namespace GNET
{

void AdapterInfo::SetDes(const std::string &str)
{
	des = StringHelper::trim(str);
}

};

