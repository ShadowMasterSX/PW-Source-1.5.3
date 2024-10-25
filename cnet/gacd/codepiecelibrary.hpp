#ifndef __GNET_CODEPIECELIBRARY_HPP
#define __GNET_CODEPIECELIBRARY_HPP

#include <map>

#include "codepiece.hpp"

#include "preparedcodefragment.hpp"
#include "xmlconfig.h"

namespace GNET
{

class CodePieceLibrary
{
	typedef std::map<std::string, CodePiece> PieceMap;
	typedef std::map<short, PieceMap> Library;
	Library m_library;

	static CodePieceLibrary s_instance;
public:
	~CodePieceLibrary() { }

	static CodePieceLibrary *GetInstance() { return &s_instance; }

	void OnUpdateConfig(const XmlConfig::Node *pRoot);

	bool IsEmpty() const { return m_library.empty(); }
	void Clear();

	void ParseCode(const PreparedCodeFragment &pcf, CodePieceVector &cpv) const;
private:
	CodePieceLibrary() { }
	CodePieceLibrary(const CodePieceLibrary &cpl);
	CodePieceLibrary& operator=(const CodePieceLibrary &cpl);
	bool operator==(const CodePieceLibrary &cpl) const;
};

};

#endif
