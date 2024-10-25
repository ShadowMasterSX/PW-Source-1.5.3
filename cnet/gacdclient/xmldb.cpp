#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

#include "xmldb.h"
#include "stringcaster.h"

void XmlDB::Save() const
{
	CheckDir();
	std::ofstream ofs(GetFileName().c_str());
	ofs << (StringCaster<XmlObject>::ToString(WriteObject()));
}

void XmlDB::Load()
{
	CheckDir();
	std::ifstream ifs(GetFileName().c_str());
	std::string buffer;
	std::string res;
	
	if( ifs.is_open() ) while( ! ifs.eof() ) { std::getline(ifs, buffer); res += buffer + XmlObject::nl; }
	LoadObject(StringCaster<XmlObject>::FromString(res));
}

void XmlDB::CheckDir() const
{
	std::string fn = GetFileName();
	std::string::size_type pos = fn.find_first_of('/');
	if( pos == std::string::npos ) return;
	while( pos != std::string::npos )
	{
		struct stat buff;
		const char *pn = fn.substr(0, pos).c_str();
		if( -1 == stat(pn, &buff) )
		{
			if( errno == ENOENT )
				mkdir(pn, 0775);
			else
				throw Exception();
		}
		else
		{
			if( !(S_ISDIR(buff.st_mode)) )
				throw Exception();
		}
		
		pos = fn.find_first_of('/', pos+1);
	}
}
