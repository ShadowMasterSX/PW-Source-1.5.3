#ifndef __MY_STRINGCASTER
#define __MY_STRINGCASTER

#include <string>

template<class T>
struct StringCaster
{
	static std::string ToString(const T& _t);
	static T FromString(const std::string &_str);
};

template<class T>
inline std::string StringCaster<T>::ToString(const T& _t)
{
	return _t.ToString();
}

template<class T>
inline T StringCaster<T>::FromString(const std::string &_str)
{
	return T().FromString(_str);
}

template<> 
inline std::string StringCaster<int>::ToString(const int& _t)
{
	char buffer[15];
	sprintf(buffer, "%d", _t);
	return std::string(buffer);
}

template<> 
inline int StringCaster<int>::FromString(const std::string &_str)
{
	return atoi(_str.c_str());
}

template<> 
inline std::string StringCaster<size_t>::ToString(const size_t& _t)
{
	char buffer[15];
	sprintf(buffer, "%d", _t);
	return std::string(buffer);
}

template<> 
inline size_t StringCaster<size_t>::FromString(const std::string &_str)
{
	return atoi(_str.c_str());
}

template<> 
inline std::string StringCaster<long>::ToString(const long& _t)
{
	char buffer[15];
	sprintf(buffer, "%d", _t);
	return std::string(buffer);
}

template<> 
inline long StringCaster<long>::FromString(const std::string &_str)
{
	return atol(_str.c_str());
}

template<> 
inline std::string StringCaster<std::string>::ToString(const std::string& _t)
{
	return _t; 
}

template<> 
inline std::string StringCaster<std::string>::FromString(const std::string &_str)
{
	return _str;
}

#endif
