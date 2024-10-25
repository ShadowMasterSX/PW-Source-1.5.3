
#include "stringhelper.hpp"
#include <libxml/xmlreader.h>

namespace GNET
{

Octets StringHelper::hexstr_to_octets(std::string str)
{
	Octets os;
	size_t tsize = str.size();
    if( tsize < 2 || tsize % 2 ) { return os; }
    os.resize(tsize/2 + 4);
    const char *p_str = str.c_str();
    unsigned char *p_os = (unsigned char*)os.begin();
    unsigned int scan_int;
    while( *p_str )
    {
        sscanf(p_str, "%02x", &scan_int);
        *p_os++ = (unsigned char)(scan_int);
        p_str += 2;
    }
    return os.resize(os.size() - 4);
}

std::string StringHelper::utf8_to_iso1(const std::string &str)
{
	unsigned char buffert[1024];
	std::string r;
    if( str.size() > 0 )
    {
        int lent = 1024;
        int inlen = str.size();
        if( 0 <= UTF8Toisolat1(buffert, &lent, (const unsigned char*)str.c_str(), &inlen))
		{
	        buffert[lent] = 0;
    	    r = std::string((const char*)buffert);
		}
    }
	return r;
}

int StringHelper::ipstr_to_int(const std::string &str)
{
	int r1 =0, r2 =0, r3 =0, r4=0;
	std::string::size_type i = 0, j =0;

	i = str.find_first_of(".", j);
	if( i == std::string::npos ) return 0;
	r1 = atoi(str.substr(j, i-j).c_str());
	j = i+1;

	i = str.find_first_of(".", j);
	if( i == std::string::npos ) return 0;
	r2 = atoi(str.substr(j, i-j).c_str());
	j = i+1;

	i = str.find_first_of(".", j);
	if( i == std::string::npos ) return 0;
	r3 = atoi(str.substr(j, i-j).c_str());
	j = i+1;

	r4 = atoi(str.substr(j).c_str());

	return (r1<<24) | (r2<<16) | (r3<<8) | r4;
}

std::string StringHelper::trim(std::string const& source, char const* delims) 
{
	std::string result(source);
	std::string::size_type index = result.find_last_not_of(delims);
	if(index != std::string::npos)
		result.erase(++index);

	index = result.find_first_not_of(delims);
	if(index != std::string::npos)
		result.erase(0, index);
	else
		result.erase();
	return result;
}

StringHelper::string_pair_vec StringHelper::parse_string_pair(const std::string &str)
{
	string_pair_vec p;
    char buffer[2048];
    strncpy( buffer, str.c_str(), std::min(sizeof(buffer)-1,str.length()) );
    buffer[sizeof(buffer)-1] = 0;
   
    char * cur = buffer;
    char * token = strchr( cur, '(' );
    while( NULL != token )
    {
        cur = token+1;
        token = strchr( cur, ',' );
        if( NULL == token ) break;
        *token = 0;
        std::string first = trim(std::string(cur));
   
        cur = token+1;
        token = strchr( cur, ')' );
        if( NULL == token ) break;
        *token = 0;
        std::string second = trim(std::string(cur));
        p.push_back(std::make_pair(first, second));

        cur = token+1;
        token = strchr( cur, '(' );
    }
    return p;
}

};

