#include <string.h>
#include "astring.h"

namespace abase
{

string::string(char *s)
{
	_isref = false;
	_length = strlen(s) ;
	_data = new char[_length + 1];
	memcpy(_data, s, _length +1);
}
string::string(const string & str)
{
	_isref = false;
	_length = str._length;
	_data = new char[_length +1];
	memcpy(_data,str._data,_length + 1);
}

string::~string()
{
	if(!_isref){
		delete _data;
	}
}

const char * string::operator=(const string &rhs)
{
	if(&rhs == this) return *this;
	if(!_isref)
	{
		delete _data;
	}

	_isref = false;
	_length = rhs._length;
	_data = new char[_length +1];
	memcpy(_data,rhs._data,_length + 1);
	return rhs;
}

const char * string::operator=(const char *rhs)
{
	if(!_isref)
	{
		delete _data;
	}
	_data	= const_cast<char*>(rhs);
	_isref 	= true;
	_length = strlen(rhs);
	return rhs;
}

char * string::operator=(char *rhs)
{
	if(!_isref)
	{
		delete _data;
	}
	_isref = false;
	_length = strlen(rhs) ;
	_data = new char[_length + 1];
	memcpy(_data, rhs, _length +1);
	return rhs;
}

const string & string::operator+=(const char *rhs)
{
	char * tmp = _data;
	int nlen = strlen(rhs);
	_data = new char[_length + nlen + 1];
	memcpy(_data,tmp,_length);
	memcpy(_data+_length,rhs,nlen);
	_length = _length + nlen;
	_data[_length] = 0;
	_isref = false;
	if(!_isref)
	{
		delete tmp;
	}
	return *this;
}

}

