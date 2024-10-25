#ifndef __CM_FIXED_STRING_H__
#define __CM_FIXED_STRING_H__

namespace abase{

template<int __length>
class fixstring
{
private:
	char _data[__length+1];
public:
	typedef const char * LPCSTR;
			
	fixstring(){_data[0] = 0;}
	fixstring(const char *__str)
	{
		int len = strlen(__str);
		if(len > __length) throw "string overflow";
		strcpy(_data,__str);
	}
	bool operator ==(const fixstring<__length> & rhs) const {
		return strcmp(_data,rhs._data) == 0;
	}

	const char * operator =(const char *rhs)
	{
		int len = strlen(rhs);
		if(len > _length) return NULL;
		return strcpy(_data,rhs);
	}

	const fixstring<__length> & operator =(const fixstring<__length> & rhs)
	{
		memcpy(_data,rhs._data,sizeof(_data));
		return *this;
	}

	operator LPCSTR() const { return _data;}

//	template<int __len> friend bool operator ==(const fixstring<__len>&, const char *);
//	template<int __len> friend bool operator ==(const char *,const fixstring<__len>&);
};

template<int __length>
inline bool operator == (const fixstring<__length> & lhs, const char *rhs){
	return strcmp(lhs,rhs) == 0;
}

template<int __length>
inline bool operator == (const char *lhs,const fixstring<__length> & rhs){
	return strcmp(lhs,rhs) == 0;
}


}

#endif

