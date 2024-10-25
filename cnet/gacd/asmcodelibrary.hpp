#ifndef __GNET_ASMCODELIBRARY_HPP
#define __GNET_ASMCODELIBRARY_HPP

#include <vector>

#include "xmlconfig.h"
#include "octets.h"

namespace GNET
{


class AsmCodeLibrary
{

	struct AsmCode
	{
    	unsigned char *pData;
		Octets m_os;			// asm code ( only udpate %eax )
	private:
    	AsmCode(const AsmCode&);
	    AsmCode& operator=(const AsmCode&);
	public:
		AsmCode() : pData(NULL) { SetCode(m_os); }
		AsmCode(const Octets &os)
		{
			SetCode(os);
		}
		AsmCode& SetCode(const Octets &os)
		{
    		Clear();
		    m_os = os;
		    pData = new unsigned char[(m_os.size() + 10)];		// code wrapper
		    pData[0] = 0x55;                                    // pushl %ebp
		    pData[1] = 0x89;                                    // movl %esp, %ebp
		    pData[2] = 0xe5;
		    pData[3] = 0xb8;                                    // movl param, %eax
		    memcpy(&pData[8], m_os.begin(), m_os.size());       // set asm code
		    pData[8+m_os.size()] = 0xc9;                        // leave
		    pData[8+m_os.size()+1] = 0xc3;                      // ret
		    return *this;
		}
	    int operator()(int param)
    	{
        	*(int*)(pData+4) = param;							// setup param
	        return ((int(*)())pData)();							// call wrapper
    	}
		void Clear() { m_os.clear(); delete[] pData; pData = NULL; }
	    ~AsmCode() { Clear(); }
	};	

	typedef std::vector<Octets> Library;
	Library m_library;

	AsmCode m_cacheCode;

	static AsmCodeLibrary s_instance;
	unsigned int m_uMaxSize;
public:
	~AsmCodeLibrary() { }

	static AsmCodeLibrary *GetInstance() { return &s_instance; }
	
	bool MakeCode(AsmCode &ac);

	void GetCode(Octets &os, int &res)
	{
		os = m_cacheCode.m_os;
		int param = res;
		res = m_cacheCode(param);
	}

	void OnUpdateConfig(const XmlConfig::Node *pRoot);
	void OnTimer();

	bool IsEmpty() const { return m_library.empty(); }
	void Clear();

private:
	AsmCodeLibrary() : m_uMaxSize(200) { }
	AsmCodeLibrary(const AsmCodeLibrary &acl);
	AsmCodeLibrary& operator=(const AsmCodeLibrary &acl);
	bool operator==(const AsmCodeLibrary &acl) const;
};

};

#endif
