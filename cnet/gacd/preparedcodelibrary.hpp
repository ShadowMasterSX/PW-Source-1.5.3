#ifndef __GNET_PREPAREDCODELIBRARY_HPP
#define __GNET_PREPAREDCODELIBRARY_HPP

#include <map>

#include "preparedcode.hpp"
#include "xmlconfig.h"

namespace GNET
{

class PreparedCodeLibrary
{
	typedef std::map<std::string, PreparedCode> Library;
	Library m_library;

	static PreparedCodeLibrary s_instance;
public:
	~PreparedCodeLibrary() { }

	static PreparedCodeLibrary *GetInstance() { return &s_instance; }

	void OnUpdateConfig(const XmlConfig::Node *pRoot);

	bool IsEmpty() const { return m_library.empty(); }
	void Clear() { m_library.clear(); }

	int MakeCode(std::string codeName, CodePieceVector &cpv,
        std::vector<std::string> params = std::vector<std::string>() ) const;


private:
	PreparedCodeLibrary() { }
	PreparedCodeLibrary(const PreparedCodeLibrary &pcl);
	PreparedCodeLibrary& operator=(const PreparedCodeLibrary &pcl);
	bool operator==(const PreparedCodeLibrary &pcl) const;
};

};

#endif
