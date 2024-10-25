#ifndef _NMD_TREE_H_
#define _NMD_TREE_H_

#include <hashtab.h>
#include "NmdChd.h"

class A3DVECTOR3;


namespace SvrCD
{
class CBrushTree;
class CNmdChd;
class CCDBrush;

typedef abase::vector<CCDBrush*>    BrushVec;
typedef abase::hashtab<BrushVec, int, abase::_hash_function>	BrushTable;


class CNmdTree
{
public:
	CNmdTree();
	~CNmdTree();
	/**
	 * \brief init the nmd tree
	 * \param[in] ext:  the tree's extent in the world space
	 * \param[out] 
	 * \return
	 * \note
	 * \warning
	 * \todo   
	 * \author kuiwu 
	 * \date 25/8/2008
	 * \see 
	 */
	bool Init(const A3DVECTOR3& ext);
	void Release();

	bool AddNpcMine(CNmdChd *pNmdChd, int tplId, int uId, const A3DVECTOR3& pos, const A3DVECTOR3& up, const A3DVECTOR3& dir);
	bool AddDynObj(CNmdChd *pNmdChd, int tplId, int uId, const A3DVECTOR3& pos, const A3DVECTOR3& up, const A3DVECTOR3& dir);
	void EnableNmd(int uId, bool enable) const;
	bool IsNmdEnabled(int uId) const;

	CBrushTree * GetBrushTree() const
	{
		return m_pBrushTree;
	}
#ifdef BMAN_EDITOR
	const	EcmTable * GetEcmTab() const
	{
		return &m_EcmTab;
	}
#endif

private:
	bool _AddNMD(CNmdChd * pNmdChd, int tplId, int uId, const  A3DVECTOR3& pos, const A3DVECTOR3& up, const A3DVECTOR3& dir, bool dyn);
	
private:
	//const CNmdChd	*	m_pNmdChd;
	CBrushTree  *	m_pBrushTree;
	BrushTable      m_BrushTab;
#ifdef BMAN_EDITOR
	EcmTable				 m_EcmTab;				 
#endif

};

}


#endif

