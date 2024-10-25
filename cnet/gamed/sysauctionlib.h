#ifndef __GNET_SYSAUCTIONLIB_H__
#define __GNET_SYSAUCTIONLIB_H__
namespace GNET
{
	// Lib function for gameserver
	bool ForwardSysAuctionOP(unsigned int type, const void * pParams, size_t param_len,object_interface obj_if);	
}
#endif
