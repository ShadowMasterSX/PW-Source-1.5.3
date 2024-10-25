#include <algorithm>

#include "gacdutil.h"
#include "stringhelper.hpp"
#include "asmcodelibrary.hpp"

namespace GNET
{

AsmCodeLibrary AsmCodeLibrary::s_instance;

void AsmCodeLibrary::Clear()
{
	m_library.clear();
}

void AsmCodeLibrary::OnTimer()
{
	MakeCode(m_cacheCode);
}

bool AsmCodeLibrary::MakeCode(AsmCode &ac)
{
	size_t n = m_library.size();
	if( n == 0 ) return true;
	Octets os;
	while( os.size() < m_uMaxSize )
	{	
		Octets &sos = m_library[rand()%n];
		os.insert(os.begin(), sos.begin(), sos.size());
	}
	ac.SetCode(os);
	return true;
}

void AsmCodeLibrary::OnUpdateConfig(const XmlConfig::Node* pRoot)
{
	DEBUG_PRINT_INIT("before reload asm code piece\n");
	Clear();
        
	const XmlConfig::Node* asmcode= pRoot->GetFirstChild("codemanager")->GetFirstChild("asmcode");
	asmcode->GetUIntAttr("max_size", &m_uMaxSize);
    XmlConfig::Nodes nodes = asmcode->GetChildren("piece");
    for(XmlConfig::Nodes::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {   
		m_library.push_back(StringHelper::hexstr_to_octets((*it)->GetAttr("data")));
    }
	MakeCode(m_cacheCode);
	DEBUG_PRINT_INIT("after reload asm code piece, total %d\n", m_library.size());
}

};

