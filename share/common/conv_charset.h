#ifndef __GNET_CONV_CHARSET_H
#define __GNET_CONV_CHARSET_H

#include <iconv.h>
#include "octets.h"
#include "conf.h"
#include "thread.h"

namespace GNET
{
	class CharsetConverter
	{
			static	Thread::Mutex	locker_iconv;

			static size_t conv_charset(iconv_t& cd,const Octets& os_from, Octets& os_to)
			{
				if ((iconv_t)-1==cd) return (size_t) -1;
				if (0==os_from.size()) return (size_t)-1;

				size_t szFrom=os_from.size();
				size_t szTo=szFrom*4,old_szTo;

				char* from=(char*)os_from.begin();
				char* to=(char*)malloc(szTo);
				char* tmp_to=to;

				old_szTo=szTo;
				if ( ((size_t) -1)==iconv(cd,&from,&szFrom,&tmp_to,&szTo) )
				{
					free(to);
					return (size_t)-1;
				}
				os_to.replace(to,old_szTo-szTo);
				free(to);
				return old_szTo-szTo;

			}
			static iconv_t& GetU2G()
			{
				static iconv_t cd=iconv_open("GBK","UCS2");
				return cd;
			}
			static iconv_t& GetG2U()
			{
				static iconv_t cd=iconv_open("UCS2","GBK");
				return cd;
			}
			static iconv_t& GetU2T()
			{
				static iconv_t cd=iconv_open("UTF-8","UCS2");
				return cd;
			}
			static iconv_t& GetT2U()
			{
				static iconv_t cd=iconv_open("UCS2","UTF-8");
				return cd;
			}
			static iconv_t& GetU2B()
			{
				static iconv_t cd=iconv_open("BIG5","UCS2");
				return cd;
			}
			static iconv_t& GetB2U()
			{
				static iconv_t cd=iconv_open("UCS2","BIG5");
				return cd;
			}
		public:
			static size_t conv_charset_u2g(const Octets& os_from, Octets& os_to)
			{
				Thread::Mutex::Scoped lock(locker_iconv);
				return conv_charset(GetU2G(),os_from,os_to);
			}
			static size_t conv_charset_u2t(const Octets& os_from, Octets& os_to)
			{
				Thread::Mutex::Scoped lock(locker_iconv);
				return conv_charset(GetU2T(),os_from,os_to);
			}
			static size_t conv_charset_t2u(const Octets& os_from, Octets& os_to)
			{
				Thread::Mutex::Scoped lock(locker_iconv);
				return conv_charset(GetT2U(),os_from,os_to);
			}
			static size_t conv_charset_g2u(const Octets& os_from, Octets& os_to)
			{
				Thread::Mutex::Scoped lock(locker_iconv);
				return conv_charset(GetG2U(),os_from,os_to);
			}

			static size_t conv_charset_u2l( Octets & os_from, Octets & os_to )
			{
				Thread::Mutex::Scoped lock(locker_iconv);
				if( 0 == strcasecmp( Conf::GetInstance()->find("common","charset").c_str(), "utf-8" ) )
					return conv_charset(GetU2T(),os_from,os_to);
				else if( 0 == strcasecmp( Conf::GetInstance()->find("common","charset").c_str(), "big5" ) )
					return conv_charset(GetU2B(),os_from,os_to);
				else
					return conv_charset(GetU2G(),os_from,os_to);
			}
			static size_t conv_charset_l2u( Octets & os_from, Octets & os_to )
			{
				Thread::Mutex::Scoped lock(locker_iconv);
				if( 0 == strcasecmp( Conf::GetInstance()->find("common","charset").c_str(), "utf-8" ) )
					return conv_charset(GetT2U(),os_from,os_to);
				else if( 0 == strcasecmp( Conf::GetInstance()->find("common","charset").c_str(), "big5" ) )
					return conv_charset(GetB2U(),os_from,os_to);
				else
					return conv_charset(GetG2U(),os_from,os_to);
			}
	};
	
	/*
	static size_t conv_charset(const char* encode_from,const char* encode_to,const Octets& os_from, Octets& os_to)
	{
		
		os_to=os_from;
		return 0;
		
		
		iconv_t cd;
		if ( ((iconv_t) -1) == (cd=iconv_open(encode_to,encode_from)) ) return (size_t)-1;
		if (0==os_from.size()) return (size_t)-1;
		
		size_t szFrom=os_from.size();
		size_t szTo=szFrom*4,old_szTo;
		
		char* from=(char*)os_from.begin();
		char* to=(char*)malloc(szTo);
		char* tmp_to=to;

		old_szTo=szTo;
		if ( ((size_t) -1)==iconv(cd,&from,&szFrom,&tmp_to,&szTo) )
		{
			free(to);
			return (size_t)-1;
		}
		os_to.replace(to,old_szTo-szTo);
		free(to);
		iconv_close(cd);		
		return old_szTo-szTo;
		
	}
	*/
};
#endif	
