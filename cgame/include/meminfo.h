#ifndef __CLIB_MEM_INFO_H__
#define __CLIB_MEM_INFO_H__

#include <stdio.h>
inline size_t GetMemTotal()
{
	FILE * file = fopen("/proc/meminfo","r");
	if(!file) return (size_t)-1;
	char buf[512];
	while(fgets(buf,sizeof(buf),file))
	{
		if(memcmp(buf,"MemTotal:",9) == 0)
		{
			size_t s = (size_t)-1;
			sscanf(buf,"MemTotal:\t%u kB",&s);
			return s;
		}
		
	}
	return (size_t) -1;
}
#endif

