#include "SMTree.h"
#include "BrushTree.h"



namespace SvrCD
{
CSMTree::CSMTree()
:m_pBrushTree(NULL)
{

}

CSMTree::~CSMTree()
{

}

bool CSMTree::Load(const char * szName)
{
	Release();
	m_pBrushTree = new CBrushTree;
	return m_pBrushTree->Load(szName);
}

void CSMTree::Release()
{
	if (m_pBrushTree)
	{
		m_pBrushTree->Release();
		delete  m_pBrushTree;
		m_pBrushTree = NULL;
	}
}


}

