#ifndef __GNET_MEMPATTERNLIBRARY_HPP
#define __GNET_MEMPATTERNLIBRARY_HPP

#include <map>
#include <deque>

#include "mempatterncode.hpp"
#include "codeprovider.hpp"
#include "xmlconfig.h"

namespace GNET
{

class MemPatternLibrary : public CodeProvider
{
	typedef std::map<int, MemPatternCode> Library;
	Library m_library;

	SendingQueue m_rawQueue;

	static MemPatternLibrary s_instance;
public:
	~MemPatternLibrary() { }

	static MemPatternLibrary *GetInstance() { return &s_instance; }

	void OnUpdateConfig(const XmlConfig::Node *pRoot);
	void UpdateSendingQueue(SendingQueue &queue) const;
	bool GetCodePieceVector(int iUserID, int iID, CodePieceVector &cpv, int iSeq, CodeResChecker *&pChecker);

	bool IsEmpty() const { return m_library.empty(); }
	void Clear() { m_library.clear(); m_rawQueue.clear(); }

private:
	MemPatternLibrary() { }
	MemPatternLibrary(const MemPatternLibrary &ml);
	MemPatternLibrary& operator=(const MemPatternLibrary &ml);
	bool operator==(const MemPatternLibrary &ml) const;
};

};

#endif
