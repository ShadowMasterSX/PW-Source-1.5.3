
#include "codepiecelibrary.hpp"
#include "gacdutil.h"

namespace GNET
{

CodePieceLibrary CodePieceLibrary::s_instance;

void CodePieceLibrary::Clear()
{
	DEBUG_PRINT_INIT("clear size %d\n", m_library.size());
	m_library.clear();
}

void CodePieceLibrary::OnUpdateConfig(const XmlConfig::Node *pRoot)
{
	DEBUG_PRINT_INIT("before reload code piece library\n");
	Clear();
        
    int index = 0;
    XmlConfig::Nodes nodes = pRoot->GetFirstChild("NetDLL")->GetFirstChild("Compile")->GetChildren("code");
    for(XmlConfig::Nodes::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {   
        std::string id = (*it)->GetAttr("id");
        std::string value = (*it)->GetAttr("value");
        if( id.empty() || value.empty() ) continue;
        std::string name = (*it)->GetAttr("symbol");
        if( name.empty() )
        {
            char buffer[30];
            sprintf(buffer,"%d",index++);
            name = id+"_@#^&@*"+std::string(buffer);
        }
		CodePiece cp(value);	
		if( !cp.IsEmpty() )
			m_library[atoi(id.c_str())][name] = cp;
    }
	DEBUG_PRINT_INIT("after reload code piece library\n");
}

void CodePieceLibrary::ParseCode(const PreparedCodeFragment &pcf, CodePieceVector &cpv) const
{
	short sType = pcf.GetType();
	if( !pcf.IsValid() || sType == PreparedCodeFragment::FT_PARAM ) return;
	Library::const_iterator it = m_library.find(pcf.GetCodeID());
	if( it == m_library.end() ) return;
	const PieceMap &pm = (*it).second;
	size_t n = pm.size();
	if( n == 0 ) return;
	if( sType == PreparedCodeFragment::FT_FIXED )
	{
		PieceMap::const_iterator it2 = pm.find(pcf.GetCodeName());
		if( it2 != pm.end() ) 
		{
			cpv.push_back((*it2).second);
			//const CodePiece &cp = (*it2).second;
			//ov.push_back(cp.GetData());
			//if( cp.IsRunType() )
			//	ss[(*it).first]++;
		}
	}
	else if( sType == PreparedCodeFragment::FT_ALL )
	{
		for(PieceMap::const_iterator it2 = pm.begin(), ie2 = pm.end(); it2 != ie2; ++it2)
		{
			cpv.push_back((*it2).second);
			//const CodePiece &cp = (*it2).second;
			//ov.push_back(cp.GetData());
			//if( cp.IsRunType() )
			//	ss[(*it).first]++;
		}
	}
	else if( sType == PreparedCodeFragment::FT_RANDOM )
	{
		PieceMap::const_iterator it2 = pm.begin();
		std::advance(it2, rand()%n);
		cpv.push_back((*it2).second);
		//const CodePiece &cp = (*it2).second;
		//ov.push_back(cp.GetData());
		//if( cp.IsRunType() )
		//	ss[(*it).first]++;
	}
}

};

