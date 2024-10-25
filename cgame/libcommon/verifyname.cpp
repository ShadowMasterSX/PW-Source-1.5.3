#include <stdlib.h>
#include <string.h>

#include "verifyname.h"

const char chartable[] = "abcdefghijklmnopqrstuvwxyz1234567890_";
int verifyname(const char * name)
{
	int len ;
	char c;
	len = strlen(name);
	if(len < 3 || len > MAX_USERNAME_LEN) return -1;
	
	if( !(*name >='A' && *name <='Z') && !(*name >='a' && *name <='z')){
		return -2;
	}
	while((c = *name++))
	{
		if(c >='A' && c <='Z') c += 'a' - 'A';
		if(!strchr(chartable,c)) return -3;
	}
	return 0;
}

