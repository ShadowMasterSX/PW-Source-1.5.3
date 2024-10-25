#ifndef __MATCHER_H
#define __MATCHER_H
#include <iconv.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#if __GNUC__ <4
	#include <pcre/pcre.h>
#else
	#include <pcre.h>
#endif

#include "thread.h"

namespace GNET
{
#define CDINVALID ((iconv_t)-1)
	using std::string;
	class Matcher
	{
		static Matcher instance;
		Thread::RWLock locker;
		iconv_t cinput;
		iconv_t cvalid;
		char errormsg[128];

		string pattern;
		pcre *regexp;

		void close()
		{
			if(cinput!=CDINVALID)
				iconv_close(cinput);
			if(cvalid!=CDINVALID)
				iconv_close(cvalid);
			if(regexp)
				pcre_free(regexp);
			cinput = CDINVALID;
			cvalid = CDINVALID;
			regexp = NULL;
		}

	public:
		Matcher() : locker("Matcher"), cinput(CDINVALID), cvalid(CDINVALID), regexp(NULL)
		{
			errormsg[0] = 0;
		}

		~Matcher()
		{
			close();
		}
		const char* GetError() { return errormsg; }
		int Load(const char* file,const char* in_code="UCS2",const char* check_code="GB2312",const char* table_code="GBK")
		{
			char *in,*out,*buf,*input;
			const char *filecharset;
			size_t ilen, olen, flen;
			string line;

			Thread::RWLock::WRScoped l(locker);

			if(!file || !file[0])
				file = "rolename.txt";
			if (access(file, R_OK) != 0)
			{
				snprintf(errormsg,128,"cannot open file %s for reading", file);
				return -1;
			}
			close();

			cinput = iconv_open("UTF8",in_code);
			if(cinput==CDINVALID)
			{
				snprintf(errormsg,128,"cannot allocates a conversion descriptor from %s to UTF8", in_code);
				return -1;
			}
			if(check_code && check_code[0])
			{
				cvalid = iconv_open(check_code,in_code);
				if(cinput==CDINVALID)
					fprintf(stderr, "Warning: cannot allocates a conversion descriptor form %s to %s\n", in_code, check_code);
			}
			if(table_code && table_code[0])
				filecharset = table_code;
			else
				filecharset = "GBK";

			std::ifstream ifs;
			ifs.open(file, std::ios::binary);
			ifs.seekg (0, std::ios::end);
			flen = ifs.tellg();
			ifs.seekg (0, std::ios::beg);
			if(flen==0)
			{
				fprintf(stderr, "Warning: %s is empty, sensitive words checking disabled\n", file);
				return 0;
			}
			if(!(input = new char[flen]))
			{
				snprintf(errormsg,128,"memory alloc failed");
				return -1;
			}
			ifs.read(input, flen);
			ifs.close();

			if(!(buf = new char[flen*4]))
			{
				snprintf(errormsg,128,"memory alloc failed");
				return -1;
			}
			in = input;
			if(*(unsigned char*)in==0xFF && *(unsigned char*)(in+1)==0xFE)
			{
				in += 2;
				if(strcasecmp(filecharset, "UCS2"))
				{
					fprintf(stderr, "Warning: table_charset should be set to UCS2\n");
					filecharset = "UCS2";
				}

			}

			iconv_t cv;
			cv = iconv_open("UTF8",filecharset);
			if(cv==CDINVALID)
			{
				snprintf(errormsg,128,"cannot allocates a conversion descriptor from %s to UTF8", table_code);
				return -1;
			}
			out = buf;
			ilen = flen;
			olen = flen*4;
			int buflen = olen;
			if(iconv(cv,&in,&ilen,&out,&olen)>=0)
			{
				buf[buflen-olen] = 0;
				std::istringstream iss(buf,std::istringstream::in);
				pattern = "";
				while(std::getline(iss, line))
				{
					line.erase(std::find(line.begin(), line.end(), '\r'), line.end());
					if(line.size()>0)
						pattern += "(" + line + ")|";
				}

				if(pattern.size()>0)
				{
					const char *error;
					int erroffset;
					ilen = pattern.size()-1;
					pattern.assign(pattern.c_str(), ilen);
					regexp = pcre_compile(pattern.c_str(),PCRE_CASELESS|PCRE_UTF8,&error,&erroffset,0); 
					if(!regexp)
					{
						snprintf(errormsg,128,"regular expression compilation failed");
						return -1;
					}
				}
				else
					snprintf(errormsg,128,"no regular expression comtained in %s", file);
			}
			else
				snprintf(errormsg,128,"faild to convert %s from %s to UTF8", file, filecharset);
			delete input;
			delete buf;
			iconv_close(cv);
			return regexp?0:-1;
		}
		static Matcher *GetInstance()
		{
			return &instance; 
		}
		bool Match(char* str, int len)
		{
			char buf[256];
			char *in, *out;
			size_t ilen, olen;

			Thread::RWLock::RDScoped l(locker);
			if(!regexp)
				return false;

			if(cvalid!=CDINVALID)
			{
				in = str;
				out = buf;
				ilen = len;
				olen = 256;
				if(iconv(cvalid,&in,&ilen,&out,&olen)==(size_t)(-1))
					return 2;
			}

			in = str;
			out = buf;
			ilen = len;
			olen = 256;
			iconv(cinput,&in,&ilen,&out,&olen);
			return pcre_exec( regexp, NULL, buf, 256-olen, 0,  PCRE_NO_UTF8_CHECK, NULL,0) >= 0;      
		}
	};

};
#endif
