#include <string.h>
#include "ASSERT.h"
#include <stdlib.h>

#include "strfunc.h"
#include "parse.h"
#include "verbose.h"


int conf_parse(char * line, parse_structure * table)
{
	int i;
	char name[512];
	char value[512];
	char * tmp;
	char * endptr;
	int len;
	
	tmp = strchr(line,'=');
	if(!tmp) 
	{
		verbosef(2,"Can not find token '=' in line %s\n",line);
		return -1;
	}
	len = tmp - line;
	memcpy(name, line, len);
	name[len] = 0;
	strcpy(value,line + len+1);
	trim(name);
	trim(value);
	if(!name[0]) 
	{
		verbosef(2,"Empty key in line %s\n",line);
		return -2;
	}
	verbosef(4,"Parsing line '%s', key '%s' value '%s'\n",line, name , value);
	i = 0;
	do
	{
		if(!table[i].tag) break;
		verbosef(5,"Sereaching key '%s' in table %d, tag: '%s'\n",name,i,table[i].tag);
		if(strcmp(table[i].tag, name) == 0)
		{	
		// match 
			switch(table[i].type)
			{
				case TYPE_CHAR:
					ASSERT(table[i].size >= sizeof(char));
					*(char*)(table[i].data) = strtol(value,&endptr,10);
					if((*endptr) || (!*value)){
						verbosef(2,"Invalid Value for type CHAR %s\n",value);
						return -2;
					}
					return 0;
				case TYPE_UCHAR:
					ASSERT(table[i].size >= sizeof(unsigned char));
					*(unsigned char*)(table[i].data) = strtol(value,&endptr,10);
					if((*endptr) || (!*value)){
						verbosef(2,"Invalid Value for type UCHAR %s\n",value);
						return -2;
					}
					return 0;
				case TYPE_SHORT:
					ASSERT(table[i].size == sizeof(short));
					*(short *)(table[i].data) = strtol(value,&endptr,10);
					if((*endptr) || (!*value)){
						verbosef(2,"Invalid Value for type SHORT %s\n",value);
						return -2;
					}
					return 0;
				case TYPE_USHORT:
					ASSERT(table[i].size == sizeof(unsigned short));
					*(unsigned short *)(table[i].data) = strtoul(value,&endptr,10);
					if((*endptr) || (!*value)){
						verbosef(2,"Invalid Value for type USHORT %s\n",value);
						return -2;
					}
					return 0;
				case TYPE_INT:
					ASSERT(table[i].size == sizeof(int));
					*(int *)(table[i].data) = strtol(value,&endptr,10);
					if((*endptr) || (!*value)){
						verbosef(2,"Invalid Value for type INT %s\n",value);
						return -2;
					}
					return 0;
				case TYPE_UINT:
					ASSERT(table[i].size == sizeof(int));
					*(unsigned int *)(table[i].data) = strtoul(value,&endptr,10);
					if((*endptr) || (!*value)){
						verbosef(2,"Invalid Value for type UINT %s\n",value);
						return -2;
					}
					return 0;
				case TYPE_FLOAT:
					ASSERT(table[i].size == sizeof(float));
					*(float *)(table[i].data) = strtod(value,&endptr);
					if((*endptr) || (!*value)){
						verbosef(2,"Invalid Value for type FLOAT %s\n",value);
						return -2;
					}
					return 0;
				case TYPE_DOUBLE:
					ASSERT(table[i].size == sizeof(double));
					*(double *)(table[i].data) = strtod(value,&endptr);
					if((*endptr) || (!*value)){
						verbosef(2,"Invalid Value for type DOUBLE %s\n",value);
						return -2;
					}
					return 0;
				case TYPE_STRING:
					strncpy((char *)table[i].data,value, table[i].size);
					return 0;
				default:
				{
					verbosef(2,"Unknow type active.., type value %d\n",table[i].type);
					return -2;
				}
			}
		}
		i ++;
	}while(1);
	verbosef(2,"Can not find matched name %s\n",name);
	return -1;
}
