#ifndef __GNET_AUCTIONSYSLIB_H
#define __GNET_AUCTIONSYSLIB_H
namespace GNET
{
	// Lib function for gameserver
	bool ForwardAuctionSysOP( unsigned int type,const void* pParams,size_t param_len,object_interface obj_if );
}
#endif
