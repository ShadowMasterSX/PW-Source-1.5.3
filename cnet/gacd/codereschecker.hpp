#ifndef __GNET_CODERESCHECKER
#define __GNET_CODERESCHECKER
#include "gacdutil.h"

namespace GNET
{
class UserSessionData;
struct WaitingCode;

struct CodeResChecker
{
    virtual void DoCheck(WaitingCode *pWC, UserSessionData *pUser, int iRes, bool &bCommit)
    {
        bCommit = false;
    }
	virtual CodeResChecker* Clone() { return new CodeResChecker(*this); }
    virtual ~CodeResChecker() { }
};

struct WaitingCode
{
    int m_iCodeID;
    int m_iTick;

    CodeResChecker *m_pChecker;

    WaitingCode() : m_iCodeID(0), m_iTick(0), m_pChecker(new CodeResChecker()) 
	{  
		DEBUG_PRINT_DEBUG("WaitingCode::WatingCode()\n");
		DEBUG_PRINT_DEBUG("default checker is %08x\n", m_pChecker);
	}
    WaitingCode(int iCodeID, CodeResChecker *pChecker = NULL )
        : m_iCodeID(iCodeID), m_iTick(0), m_pChecker(pChecker) 
	{ 
		DEBUG_PRINT_DEBUG("in WaitingCode::WatingCode(codeid, pchecker)\n");
		DEBUG_PRINT_DEBUG("after construct, checker is %08x\n", m_pChecker);
		if( NULL == m_pChecker ) 
			m_pChecker = new CodeResChecker(); 
	}
    ~WaitingCode() 
	{ 
		DEBUG_PRINT_DEBUG("in WaitingCode::~WatingCode()\n");
		DEBUG_PRINT_DEBUG("before delete checker, checker is %08x\n", m_pChecker);
		delete m_pChecker; 
	}

    int Tick() { return ++m_iTick; }
	WaitingCode(const WaitingCode &wc) : m_iCodeID(wc.m_iCodeID), m_iTick(wc.m_iTick)
        , m_pChecker(new CodeResChecker(*wc.m_pChecker)) 
	{ 
		DEBUG_PRINT_DEBUG("in WaitingCode::WatingCode(WatingCode&)\n");
	}
    WaitingCode& operator=(const WaitingCode &wc)
    {
		DEBUG_PRINT_DEBUG("in WaitingCode::operator=(WatingCode&), this 0x%08x, other 0x%08x\n"
			, this, &wc);
		DEBUG_PRINT_DEBUG("old checker is %08x, new checker is %08x\n", m_pChecker, wc.m_pChecker);
        if( this != &wc )
		{
        	m_iCodeID = wc.m_iCodeID;
	        m_iTick = wc.m_iTick;
    	    delete m_pChecker; 
			m_pChecker = wc.m_pChecker->Clone();
		}
		DEBUG_PRINT_DEBUG("new checker is %08x\n", m_pChecker);
        return *this;
    }
};

struct CodeResCheckerWithAnswer : public CodeResChecker
{
    int m_iAnswer;
    CodeResCheckerWithAnswer(int iAnswer) : m_iAnswer(iAnswer) { }
    virtual void DoCheck(WaitingCode *pWC, UserSessionData *pUser, int iRes, bool &bCommit)
    {
        bCommit = m_iAnswer != iRes;
    }
	virtual CodeResChecker* Clone() { return new CodeResCheckerWithAnswer(*this); }
    virtual ~CodeResCheckerWithAnswer() { }
};

};

#endif
