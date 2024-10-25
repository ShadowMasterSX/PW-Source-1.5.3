/* @file: sellpointsyslib.h
 * @description: interface for gameserver to forward sellpoint request to delivery or gamedbd
 * 
 * @date: 2006-1-9
 * @author:liping
 */  

#ifndef __GNET_SELLPOINTSYSLIB_H
#define __GNET_SELLPOINTSYSLIB_H
namespace GNET
{
	bool ForwardSellPointSysOP( unsigned int type,const void* pParams,size_t param_len,object_interface obj_if );
}
#endif
