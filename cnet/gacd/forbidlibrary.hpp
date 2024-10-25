#ifndef __GNET_FORBIDLIBRARY_HPP
#define __GNET_FORBIDLIBRARY_HPP

#include <map>
#include <deque>

#include "forbidcode.hpp"
#include "codeprovider.hpp"
#include "xmlconfig.h"

namespace GNET
{

struct CodeResCheckerFromForbid : public CodeResChecker
{
    virtual void DoCheck(WaitingCode *pWC, UserSessionData *pUser, int iRes, bool &bCommit);
	virtual CodeResChecker* Clone() { return new CodeResCheckerFromForbid(*this); }
    virtual ~CodeResCheckerFromForbid() { }
};

class ForbidLibrary : public CodeProvider
{
	typedef std::map<int, ForbidCode> Library;
	Library m_library;

	SendingQueue m_rawQueue;

	static ForbidLibrary s_instance;
public:
	~ForbidLibrary() { }

	static ForbidLibrary *GetInstance() { return &s_instance; }

	void OnUpdateConfig(const XmlConfig::Node *pRoot);
	void UpdateSendingQueue(SendingQueue &queue) const;
	bool GetCodePieceVector(int iUserID, int iID, CodePieceVector &cpv, int iSeq, CodeResChecker *&pChecker);

	bool IsEmpty() const { return m_library.empty(); }
	void Clear() { m_library.clear(); m_rawQueue.clear(); }

	bool CheckRes(int iID, int iRes) const;

private:
	ForbidLibrary() { }
	ForbidLibrary(const ForbidLibrary &fl);
	ForbidLibrary& operator=(const ForbidLibrary &fl);
	bool operator==(const ForbidLibrary &fl) const;
};

};

#endif
