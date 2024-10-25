#ifndef __GNET_PSHOP_SYSLIB_H__
#define __GNET_PSHOP_SYSLIB_H__

namespace GNET
{
	//Lib function for gameserver
	bool ForwardPShopSysOP(unsigned int type,const void* pParams,size_t param_len,object_interface player);
	bool ForwardPShopSysOPTest(const void* pParams,size_t param_len,object_interface player);
	
};

#endif
