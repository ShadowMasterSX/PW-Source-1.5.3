#ifndef __ABASE_STRING_H__
#define __ABASE_STRING_H__

namespace abase{
class string
{
private:
	char* 	_data;
	int   	_length;
	bool	_isref;
public:
	typedef const char* LPCSTR;
	typedef char* LPSTR;
	string(char *s);
	string(const string &str);
	string(const char *s){
		_data	= const_cast<char*>(s);
		_isref 	= true;
		_length = strlen(s);
	}
	~string();
	int	length() const { return _length;}

//operator here
			operator LPCSTR() const {return _data;}
	bool 		operator==(const string &rhs) const { return !strcmp(*this,rhs);}
	const char * 	operator=(const string &rhs);
	const char * 	operator=(const char *rhs);
	char * 		operator=(char *rhs);
	const string & operator+=(const char *rhs);
};
inline bool operator==(const char * lhs, const string &rhs)
{
	return !strcmp(lhs,rhs);
}

inline bool operator==(const string &lhs,const char * rhs )
{
	return !strcmp(lhs,rhs);
}


}
#endif
