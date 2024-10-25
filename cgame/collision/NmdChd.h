/*
 * FILE: NmdChd.h
 *
 * DESCRIPTION: npc,mine and dyn chd manager
 *
 * CREATED BY: wang kuiwu, 2008/5/16
 *
 * HISTORY: 
 *
 * Copyright (c) 2008 Archosaur Studio, All Rights Reserved.
 */
 
#ifndef _NMD_CHD_H_
#define _NMD_CHD_H_

#include <vector.h>
#include <hashtab.h>

#include "ABaseDef.h"

namespace SvrCD
{

class CConvexHullData;

typedef abase::vector<CConvexHullData*>    ChdVec;
typedef abase::hashtab<ChdVec, int, abase::_hash_function>	EcmTable;
	
class CNmdChd
{
public:
//	typedef abase::vector<CConvexHullData*>    ChdVec;
//	typedef abase::hashtab<ChdVec, int, abase::_hash_function>	EcmTable;

public:
	CNmdChd();
	~CNmdChd();
	bool Load(const char * path);
	void Release();
	const ChdVec * GetNpcMine(int id) const;
	const ChdVec * GetDynObj(int id) const;
	DWORD		   GetElementPrgVer() const
	{
		return m_ElementPrgVer;
	}
	DWORD			GetElementDatVer() const
	{
		return m_ElementDatVer;
	}
private:
	//void _ReleaseChdVec(ChdVec& chdVec);
	//void _ReleaseEcmTab(EcmTable& ecmTab);
	void _LoadEcmTab(FILE * fp, EcmTable& ecmTab);
	const ChdVec * _GetChdVec(const EcmTable& ecmTab, int id) const;
private:
	DWORD	m_ElementPrgVer;  //element data prg version
	DWORD    m_ElementDatVer;
	EcmTable		m_NMTab;
	EcmTable        m_DynTab;

};

//funcs
void ReleaseChdVec(ChdVec& chdVec);
void ReleaseEcmTab(EcmTable& ecmTab);
}
#endif
