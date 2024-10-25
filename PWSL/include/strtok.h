/*
        strok.h         string token
        author:         cuiming
        Date:           2002/1/29
*/

#ifndef __ABASE_STRTOK_H__
#define __ABASE_STRTOK_H__

namespace abase{
class strtok
{
private:
        enum {BUF_SIZE = 128};
	const char * _srcstr;
        const char * _delim;
        const char * _last;
        char _buf[BUF_SIZE];
public:
       strtok(){};
       strtok(const char * src,const char *delim);
       int reset(const char * src,const char *delim);
       inline int offset(){ 
       		if(!_last) return -1;
		return _last - _srcstr;
       }
       const char * token(){return token(_buf,BUF_SIZE);}
       const char * token(char * output, int len);
       char * org_token();	//BE CARE: this function will try to change source string
};
}
#endif
