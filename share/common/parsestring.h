
#ifndef __GNET_PARSESTRING_HH
#define __GNET_PARSESTRING_HH

#include <string>
#include <vector>

namespace GNET
{

	static bool ParseStrings( const std::string & src, std::vector<std::string> & result )
	{
		if( src.length() <= 0 )
			return false;

		char * delim = ",";
		char * buffer = new char[src.length()+1];
		if( NULL == buffer )
			return false;
		memcpy( buffer, src.c_str(), src.length() );
		buffer[src.length()] = 0;

		char * token = strtok( buffer, delim );
		while( NULL != token )
		{
			result.insert( result.end(), token );
			token = strtok( NULL, delim );
		}

		delete [] buffer;
		return true;
	}

};
#endif

