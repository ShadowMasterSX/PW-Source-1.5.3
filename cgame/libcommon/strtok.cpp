#include <string.h>
#include "strtok.h"

abase::strtok::strtok(const char *src, const char *delim)
{
        memset(_buf,0,sizeof(_buf));
        reset(src,delim);
}

int abase::strtok::reset(const char * src, const char * delim)
{
        _srcstr = src;
        _delim = delim;
        _last = src;
        return 0;
}

const char * abase::strtok::token(char * output, int len)
{
 const char * tok;
 const char * spanp;
 const char * s;
 int c, sc;

 if(*_last == '\0') return NULL;
 s = _last;

 /*
   * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
 */
 
 cont:
 c = *s++;
 for (spanp = _delim; (sc = *spanp++) != 0;) {
     if (c == sc)
     goto cont;
 }
 if (c == 0) {                  /* no non-delimiter characters */
    _last = s - 1;
    return (NULL);
 }
 tok = s - 1;
 /*
 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
 * Note that delim must have one NUL; we stop if we see that, too.
 */
 
 while(1)
 {
  c = *s++;
  spanp = _delim;
  do {
     if ((sc = *spanp++) == c) {
        if(c)
        {
         int tmp = s - tok - 1;
         tmp = tmp > len-1?len-1:tmp;
         memcpy(output,tok,tmp);
         output[tmp] = 0;
	 _last = s;
	 return (output);
        }
	else
	{
	 output[len-1] = 0;
	 strncpy(output,tok,len-1);
	 _last = s - 1;
	  return (output);
	}
     }
  } while(sc != 0);
 }
}

char * abase::strtok::org_token()
{
 char * tok;
 const char * spanp;
 char * s;
 int c, sc;

 if(*_last == '\0') return NULL;
 s = (char *)_last; //be ware

 /*
   * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
 */
 
 cont:
 c = *s++;
 for (spanp = _delim; (sc = *spanp++) != 0;) {
     if (c == sc)
     goto cont;
 }
 if (c == 0) {                  /* no non-delimiter characters */
    _last = s - 1;
    return (NULL);
 }
 tok = s - 1;
 /*
 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
 * Note that delim must have one NUL; we stop if we see that, too.
 */
 
 while(1)
 {
  c = *s++;
  spanp = _delim;
  do {
     if ((sc = *spanp++) == c) {
        if(c)
        {
	 s[-1] = 0;
	  _last = s;
	 return (tok);
        }
	else
	{
	  _last = s - 1;
	  return (tok);
	}
     }
  } while(sc != 0);
 }
}
/*
#include <stdio.h>
#include <time.h>
int main()
{
char org[] = "faint 1 2 3 4 5 6 7";
char buf[10];
 const char *p;
 abase::strtok sk;
 int i;
 printf("%d/%d\n",clock(),CLOCKS_PER_SEC);
 for(i = 0; i < 1000000;i++){
 	strcpy(org,"faint 1 2 3 4 5 6 7");
	 sk.reset(org," ,");
	 while((p = sk.org_token()))
	 {
	 }
 }
 printf("%d\n",clock());
 strcpy(org,"faint 1 2 3 4 5 6 7");
 sk.reset(org," ,");
 while((p = sk.org_token()))
 {
 	printf("'%s'  ",p);
 }
 printf("\n");
 return 0;
}*/
