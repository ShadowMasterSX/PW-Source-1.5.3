/*
 *	一个简单的解析字符串类， 用于解析 a,b,c,d,e,f这样的字符串
 *	作者，时间：未知
*/
#ifndef __GNET_PARSESTRING_HH
#define __GNET_PARSESTRING_HH

#include <string>
#include <vector>
#include <string.h>

namespace GNET
{

	static bool ParseStrings( const std::string & src, std::vector<std::string> & result )
	{
		if( src.length() <= 0 )
			return false;

		const char * delim = ", \t";
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

