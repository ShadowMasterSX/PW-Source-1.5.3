
#include <map>
#include <iostream>
#include "byteorder.h"
#include <assert.h>
#include "marshal.h"

using namespace GNET;

class A: public Marshal
{
public:
	double x;
	unsigned int y;
	A(double a, int b): x(a), y(b) {} 
	OctetsStream& marshal(OctetsStream & os) const
	{
		return os << x << y;
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		printf("unmarshal a");
		return os >> x >> y;
	}

};

typedef std::map<char, double> Map;
class B: public Marshal
{
public:
	A a;
	Map z;
	B(double x, int y, Map b): a(x, y), z(b) {}
	OctetsStream& marshal(OctetsStream & os) const
	{
		return os << MarshalContainer(z) << a;
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		printf("unmarshal b\n");
		return os >> MarshalContainer(z) >> a;
	}
};

int main()
{
	{
	#ifdef TEST_ASSERT
	Marshal::OctetsStream ostemp;
	std::basic_string<wchar_t> x = L"asdf";
	std::basic_string<short> y;
	y.append(1,0x1234);
	ostemp << x << y;
	std::basic_string<wchar_t> x2;
	std::basic_string<short> y2;
	ostemp >> x2 >> y2;
	assert(x == x2);
	assert(y == y2);
	#endif
	}

	{
	Marshal::OctetsStream ostemp;
	std::basic_string<char> x = "asdfasdf";
	std::basic_string<char> x2;
	ostemp << x;
	ostemp >> x2;
	assert(x == x2);
	}
	Marshal::OctetsStream ostemp;
	std::string str = "this is a test";
	ostemp << str;
	ostemp.push_byte(str.c_str(),str.length());
	std::string str2;
	ostemp << str;
	ostemp >> str2;


	char buffer[64];
	memset(buffer,0,sizeof(buffer));
	ostemp.pop_byte(buffer,str.length());
	printf( "str=%s,str2=%s\n", str.c_str(), str2.c_str() );
	printf( "str=%s,buffer=%s\n", str.c_str(), buffer );

	Map m1, m2;
	m1[1]=1.1;
	m1[2]=3.14;
	m1[3]=2.718;
	m1[254]=1.718;
	B a(1.2, 2, m1), b(3.3, 0, m2);
	Marshal::OctetsStream os;
	a.marshal(os);
	Octets o;
	o = os;
	try{	
	Marshal::OctetsStream oo(o);
	Marshal::OctetsStream ooo(oo);
	Marshal::OctetsStream oooo;
	oooo = ooo;
	b.unmarshal(oooo);
	std::cout << b.a.x << " " << b.a.y << std::endl;
	printf("middle \n");
	for (Map::const_iterator i = b.z.begin(); i != b.z.end(); ++i)
		std::cout << (int)(*i).first << "->" << (*i).second << std::endl;
	}catch(Marshal::Exception & e)
	{
		std::cout<<"exception occured :"<<std::endl;
	}
}
