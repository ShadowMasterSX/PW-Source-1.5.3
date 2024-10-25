#ifndef __GNET_DEBUGCODELibrary_HPP
#define __GNET_DEBUGCODELibrary_HPP

#include <map>
#include <deque>

#include "debugcode.hpp"
#include "codeprovider.hpp"
#include "gacdutil.h"
#include "xmlconfig.h"

namespace GNET
{

class DebugCodeLibrary : public CodeProvider
{
	typedef std::map<int, DebugCode> Library;
	Library m_library;

	SendingQueue m_rawQueue;

	enum BindCodeID { BCI_SESSION = 9999 };
	int MakeSessionID(int iUserID) { return (0x12567843^(USERID2ACCOUNTID(iUserID))); }

	static DebugCodeLibrary s_instance;
public:

	static DebugCodeLibrary *GetInstance() { return &s_instance; }

	void OnUpdateConfig(const XmlConfig::Node *pRoot);
	void UpdateSendingQueue(SendingQueue &queue) const;
	bool GetCodePieceVector(int iUserID, int iID, CodePieceVector &cpv, int iSeq, CodeResChecker *&pChecker);

	bool IsEmpty() const { return m_library.empty(); }
	void Clear() { m_library.clear(); m_rawQueue.clear(); }

private:
	DebugCodeLibrary() {  }
	DebugCodeLibrary(const DebugCodeLibrary &dcl);
	DebugCodeLibrary& operator=(const DebugCodeLibrary &dcl);
	bool operator==(const DebugCodeLibrary &dcl) const;
};

};

#endif
