#ifndef __GNET_FACTIONLIB_H
#define __GNET_FACTIONLIB_H
#include "octets.h"
//class object_interface;
namespace GNET
{
	//Octets GenerateOperParams(int optype,const Octets& clientParams,object_interface obj_if,bool& retcode);
	Octets GenFOperParams(int optype,const Octets& clientParams,bool& retcode);
};
#endif
