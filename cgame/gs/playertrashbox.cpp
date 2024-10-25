#include <openssl/md5.h>
#include "string.h"
#include "world.h"
#include "playertrashbox.h"

//nothing to do
void
player_trashbox::SetPassword(const char * str, size_t size)
{
	if(size == 0) 
	{
		_has_passwd = false;
		return;
	}
	MD5((unsigned char*)str,size,_passwd);
	_has_passwd = true;
}

void
player_trashbox::SetPasswordMD5(const char * str, size_t size)
{
	if(size == 0) 
	{
		_has_passwd = false;
		return;
	}
	ASSERT(size == 16);
	memcpy(_passwd,str,16);
	_has_passwd = true;
}

bool 
player_trashbox::CheckPassword(const char * str, size_t size)
{
	if(!size && !_has_passwd) return true;
	if(!size && _has_passwd) return false;
	unsigned char md5[16];
	MD5((unsigned char*)str,size,md5);
	return memcmp(md5,_passwd,16) == 0;
}
	
