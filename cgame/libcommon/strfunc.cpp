#include <string.h>
#include "ASSERT.h"
#include "strfunc.h"


char * trim(char * str)
{
	return trimleft(trimright(str));
}

char * trimleft(char * str)
{
	char * tmpstr;
	ASSERT(str);
	if(!(*str)) return str;
	tmpstr = str;
	while(*tmpstr)
	{
		if(*tmpstr == ' ' || *tmpstr == '\n' || *tmpstr == '\r' || *tmpstr == '\t')
		{
			tmpstr ++;
		}
		else
			break;
	}
	if(tmpstr == str) return str;
	strcpy(str,tmpstr);
	return str;
}

char * trimright(char *str)
{
	char * tmpstr;
	ASSERT(str);
	if(!(*str)) return str;

	tmpstr = str + strlen(str) - 1;
	while(tmpstr > str)
	{
		if(*tmpstr == ' ' || *tmpstr == '\n' || *tmpstr == '\r' || *tmpstr == '\t')
		{
			*tmpstr = 0;
			tmpstr --;
		}
		else
			break;
	}
	return str;
}

char * lowerstring(char *str)
{
	char * tmp = str;
	while(*tmp)
	{
		if(*tmp <= 'Z' && *tmp >='A') *tmp += 'z' - 'Z';
		tmp ++;
	}
	return  str;
}

char * upperstring(char *str)
{
	char * tmp = str;
	while(*tmp)
	{
		if(*tmp <= 'z' && *tmp >='a') *tmp -= 'z' - 'Z';
		tmp ++;
	}
	return  str;
}

char * uppernstring(char * str, size_t size)
{
	char * tmp = str;
	for(size_t i = 0; i < size; i++,tmp++)
	{
		if(*tmp) break;
		if(*tmp<='z' && *tmp>='a') *tmp-= 'z'-'Z';
	}
	return str;
}

char * lowernstring(char * str, size_t size)
{
	char * tmp = str;
	for(size_t i = 0; i < size; i++,tmp++)
	{
		if(*tmp) break;
		if(*tmp <= 'Z' && *tmp >='A') *tmp += 'z' - 'Z';
	}
	return str;
}
