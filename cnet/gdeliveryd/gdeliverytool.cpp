#include "gdeliverytool.h"
#include "matcher.h"
#include <string.h>

using namespace GNET;

int DELIVERY_TOOL::OnFilterMode(int argc, char *argv[])
{
       if(argc >= 4 ) // only for filters test
       {
               const size_t orig_size = 128;
               size_t ilen = orig_size; size_t olen = orig_size;
               char name[orig_size] = {}; 
               char badname[orig_size] = {};
               char *in; char *out;
 
               if( 0 == strcmp("nametest",argv[2]))
               {
                       iconv_t cv = iconv_open("UCS2","GBK");
                       strncpy(name,argv[3],orig_size);
                       in = name; out = badname;
                       iconv(cv,&in,&ilen,&out,&olen);

                       if(Matcher::GetInstance()->Match(badname,orig_size))
                       {
                               printf("BAD NAME TEST MATCH!\n");
                       }
                       else
                       {
                               printf("BAD NAME TEST NO MATCH!\n");
                       }
 
                       iconv_close(cv);
                       return (1);
               }
               else if( 0 == strcmp("filtertest",argv[2]))
               {
                       char* input ;char *buf;
                       std::ifstream ifs;
                       ifs.open(argv[3], std::ios::binary);
                       ifs.seekg (0, std::ios::end);
                       size_t  flen = ifs.tellg();
                       ifs.seekg (0, std::ios::beg);
 
                       if(flen==0)
                       {
                               printf("Warning: %s is empty, sensitive words checking disabled\n",argv[3] );
                               return (-1);
                       }
                       if(!(input = new char[flen]))
                       {
                               printf("memory alloc failed");
                               return (-2);
                       }
                       ifs.read(input, flen);
                       ifs.close();

                       if(!(buf = new char[flen*4]))
                       {
                               printf("memory alloc failed");
                               return (-3);
                       }
                       in = input;
 
                       iconv_t cv;
                       cv = iconv_open("UTF8",argc == 5 ? argv[4] : "UCS2");
                       if(cv==CDINVALID)
                       {
                               printf("cannot allocates a conversion descriptor from %s to UTF8",argc == 5 ? argv[4] : "UCS2" );
                               return (-4);
                       }
                       out = buf;
                       ilen = flen;
                       olen = flen*4;
                       if(iconv(cv,&in,&ilen,&out,&olen)>=0)
                       {
                               iconv_close(cv);
                               cv = iconv_open("UCS2","UTF8");
 
                               string line;
                               std::istringstream iss(buf,std::istringstream::in);
                               int match_count =0;
                               while(std::getline(iss, line))
                               {
                                      line.erase(std::find(line.begin(), line.end(), '\r'), line.end());

                                      if(line.size()>0)
                                      {
                                               strncpy(name,line.c_str(),orig_size);
                                               ilen = orig_size; olen = orig_size;
                                               in = name; out = badname;
                                               iconv(cv,&in,&ilen,&out,&olen);
 
                                               if(Matcher::GetInstance()->Match(badname,orig_size))
                                               {
                                                   printf("%s \n",name);   
						   ++match_count;
                                               }
                                       }

                               }

                               printf("BAD NAME TOTAL MATCH %d TIMES!\n",match_count);
                       }
                       iconv_close(cv);
                       return (2);
 
               }
       }

       return (0);
}

