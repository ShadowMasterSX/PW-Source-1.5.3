#ifndef __GNET_MAILSYSLIB_H
#define __GNET_MAILSYSLIB_H
namespace GNET
{
	// Lib function for gameserver
	bool ForwardMailSysOP( unsigned int type,const void* pParams,size_t param_len,object_interface obj_if );
	void TestMassMail(unsigned char type,object_interface obj_if);
}
#endif
