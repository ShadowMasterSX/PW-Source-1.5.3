#ifndef __GNET_PREPAREDCODE_HPP
#define __GNET_PREPAREDCODE_HPP

#include <vector>
#include <list>

#include "preparedcodefragment.hpp"
#include "codepiece.hpp"

namespace GNET
{

class PreparedCode
{
	int m_iCodeID;
	struct Node
	{
		enum NodeType { NT_COMPLETE = 0, NT_RANDOM = 1, NT_PARAM = 2 };
		short m_sType;
		short m_sCodeID;
		PreparedCodeFragment m_pcf;
		CodePieceVector m_cpv;
	};
	typedef std::list<Node> ParsingList;
	ParsingList m_parsingList;
	CodePieceVector m_finalCode;
	bool operator==(const PreparedCode &pc) const; 
public:
	PreparedCode() { }
	PreparedCode(int iCodeID, const std::string &str) : m_iCodeID(iCodeID) { Parse(str); }

	int GetCodeID() const { return m_iCodeID; }
	void Parse(const std::string &str);
	void Clear() { m_parsingList.clear(); m_finalCode.clear(); }

	void PreBuild(const std::vector<PreparedCodeFragment> &pcfs);
	void MakeCode(CodePieceVector &ov , std::vector<std::string> params = std::vector<std::string>() ) const;

};

};

#endif
