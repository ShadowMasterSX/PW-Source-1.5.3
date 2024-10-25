#ifndef _SM_TREE_H_
#define _SM_TREE_H_

//static model tree
namespace SvrCD
{
class CBrushTree;


class CSMTree
{
public:
	CSMTree();
	~CSMTree();
	bool  Load(const char * szName);
	void  Release();
	CBrushTree * GetBrushTree() const
	{
		return m_pBrushTree;
	}

private:
	CBrushTree * m_pBrushTree;
};

}

#endif

