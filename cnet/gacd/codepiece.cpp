
#include <sstream>
#include <iomanip>
#include "gacdutil.h"
#include "codepiece.hpp"

namespace GNET
{

void CodePiece::SetData(const Octets &os)
{
	if( os.size() >= sizeof(Header) )
		m_data = os;
	else
		DEBUG_PRINT_CODE("assert failed\n");
}

void CodePiece::SetData(const std::string &str)
{ 
	Octets os;
	os.resize(str.size()/2 + 4);
	const char *p_str = str.c_str();
	unsigned char *p_os = (unsigned char*)os.begin();
	unsigned int scan_int;
	while( *p_str )
	{
		sscanf(p_str, "%02x", &scan_int);
		*p_os++ = (unsigned char)(scan_int);
		p_str += 2;
	}
	SetData(os.resize(os.size() - 4));
}

CodePiece& CodePiece::PatchData(int iPos, const void *pValue, unsigned short usSize)
{
	m_data.insert(((unsigned char*)m_data.begin())+iPos, pValue, usSize);
	GetHeader()->m_usSize += usSize;
	return *this;
}

CodePiece& CodePiece::PatchInt(int iPos, int iValue)
{
	DEBUG_PRINT_CODE("codepiece patch int, pos %d, patch %d, org %d\n", iPos, iValue, *(int*)((unsigned char*)m_data.begin() + iPos));
	*(int*)((unsigned char*)m_data.begin() + iPos) = iValue;
	return *this;
}
CodePiece& CodePiece::PatchShort(int iPos, short sValue)
{
	*reinterpret_cast<short*>((static_cast<unsigned char*>(m_data.begin()) + iPos)) = sValue;
	return *this;
}

CodePiece& CodePiece::ChangeID(short sID)
{
	GetHeader()->m_sID = sID;
	return *this;
}

void CodePiece::MakeIntDataPiece(short sID, int iValue)
{
	m_data.resize(sizeof(Header)+sizeof(int)+1);
	GetHeader()->m_usSize = static_cast<unsigned short>(m_data.size());
	GetHeader()->m_sID = sID;
	GetHeader()->m_sType = CPT_DATA;
	*static_cast<int*>(InnerDataBegin()) = iValue;
}

void CodePiece::SetData(short sID, const std::string &str, bool bHex)
{
	if( !bHex )
	{
		m_data.resize(str.size() + sizeof(Header) + 1);
		GetHeader()->m_usSize = static_cast<unsigned short>(m_data.size());
		GetHeader()->m_sID = sID;
		GetHeader()->m_sType = CPT_DATA;
		memcpy(InnerDataBegin(), str.c_str(), str.size() + 1);

	}
	else
	{
		m_data.resize(str.size()/2 + sizeof(Header));
		GetHeader()->m_usSize = static_cast<unsigned short>(m_data.size());
		GetHeader()->m_sID = sID;
		GetHeader()->m_sType = CPT_DATA;
		Octets o = CodePiece(str).GetData();
		memcpy(InnerDataBegin(), o.begin(), o.size());
	}
}

std::string CodePiece::GetStr() const
{ 
	std::ostringstream strbuf;
    for(const unsigned char* pc=(const unsigned char*)m_data.begin(); pc!=(const unsigned char*)m_data.end() ;pc++)
    {
        strbuf<<std::setfill('0')<<std::setw(2)<<std::hex<<(int)(*pc);
    }
    return strbuf.str();
}

};

