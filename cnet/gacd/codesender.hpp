#ifndef __GNET_CODESENDER_HPP
#define __GNET_CODESENDER_HPP

#include <string>
#include <vector>

#include "usersessiondata.hpp"
#include "codereschecker.hpp"

namespace GNET
{

class CodeSender
{
	std::string m_codeName;
	std::vector<std::string> m_params;
	CodeResChecker *m_pChecker;
public:
	CodeSender(std::string codeName, const std::vector<std::string> &params
		, CodeResChecker *pChecker )
		: m_codeName(codeName), m_params(params), m_pChecker(pChecker) { }
	~CodeSender() { delete m_pChecker; }

	std::string GetCodeName() const { return m_codeName; }
	std::vector<std::string> GetParams() const { return m_params; }

	void operator() (UserSessionData *data);

};

};

#endif
