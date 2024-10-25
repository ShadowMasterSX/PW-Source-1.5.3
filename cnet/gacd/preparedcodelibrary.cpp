
#include "preparedcodelibrary.hpp"

namespace GNET
{

PreparedCodeLibrary PreparedCodeLibrary::s_instance;

void PreparedCodeLibrary::OnUpdateConfig(const XmlConfig::Node *pRoot)
{
	Clear();
	const XmlConfig::Node *manager = pRoot->GetFirstChild("codemanager");
    XmlConfig::Nodes nodes = manager->GetChildren("precodes");

    for(XmlConfig::Nodes::const_iterator it = nodes.begin(), ie = nodes.end(); it != ie; ++it)
    {
        std::string sname = (*it)->GetAttr("name");
        std::string value = (*it)->GetAttr("value");
        if( sname.empty() || value.empty() ) continue;
		m_library[sname] = PreparedCode(atoi((*it)->GetAttr("ref").c_str()), value);
    }
        
}

int PreparedCodeLibrary::MakeCode(std::string codeName, CodePieceVector &cpv
        , std::vector<std::string> params ) const
{
	Library::const_iterator it = m_library.find(codeName);
	if( it == m_library.end() ) return 0;
	(*it).second.MakeCode(cpv, params);
	return (*it).second.GetCodeID();
}

};

