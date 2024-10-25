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
 
#include "NmdChd.h"
#include "ConvexHullData.h"

namespace SvrCD
{
//const var
const DWORD NMD_FILE_MAGIC = (DWORD)(('n'<<24)| ('m'<<16)|('d'<<8)|('f'));
const DWORD NMD_FILE_VERSION = 1;

//func
void ReleaseChdVec(ChdVec& chdVec)
{
	ChdVec::iterator  it;
	for (it = chdVec.begin(); it != chdVec.end(); ++it)
	{
		delete *it;
	}

	chdVec.clear();
}

void ReleaseEcmTab(EcmTable& ecmTab)
{
	EcmTable::iterator it;
	for (it = ecmTab.begin(); it != ecmTab.end(); ++it)
	{
		ReleaseChdVec(*it.value());
	}
	ecmTab.clear();
}

//NmdChd implement
CNmdChd::CNmdChd()
:m_NMTab(128),
m_DynTab(128)
{
	m_ElementPrgVer = 0;
	m_ElementDatVer = 0;
}

CNmdChd::~CNmdChd()
{
	
}

void CNmdChd::_LoadEcmTab(FILE * fp, EcmTable& ecmTab)
{
	int e_count;
	int i, j;
	
	fread(&e_count, sizeof(int), 1, fp);
	for (i = 0; i < e_count; ++i)
	{
		int id;
		fread(&id, sizeof(int), 1, fp);
		int c_count;
		fread(&c_count, sizeof(int), 1, fp);
		ChdVec  chdVec;
		for (j = 0; j < c_count; ++j)
		{
			CConvexHullData * chd = new CConvexHullData;
			chd->LoadBinaryData(fp);
			chdVec.push_back(chd);
		}
		ecmTab.put(id, chdVec);
	}

}

bool CNmdChd::Load(const char * path)
{
	FILE * fp = fopen(path, "rb");
	if(!fp)
	{
		return false;
	}
	DWORD flag;
	fread(&flag, sizeof(DWORD), 1, fp);

	if( flag != NMD_FILE_MAGIC)
	{
		//note: log it?
		fclose(fp);
		return false;	
	}
	fread(&flag, sizeof(DWORD), 1, fp);
	if( flag != NMD_FILE_VERSION)
	{
		//note: log it?
		fclose(fp);
		return false;	
	}
	
	fread(&flag, sizeof(DWORD), 1, fp);
	m_ElementPrgVer = flag;
	fread(&flag, sizeof(DWORD), 1, fp);
	m_ElementDatVer = flag;

	_LoadEcmTab(fp, m_NMTab);
	_LoadEcmTab(fp, m_DynTab);

	fclose(fp);
	return true;
}






void CNmdChd::Release()
{
	ReleaseEcmTab(m_NMTab);
	ReleaseEcmTab(m_DynTab);
}

const ChdVec * CNmdChd::_GetChdVec(const EcmTable& ecmTab, int id) const
{
	EcmTable::const_iterator it =	ecmTab.find(id);

	if (it == ecmTab.end())
	{
		return NULL;
	}

	return it.value();
	
}

const ChdVec * CNmdChd::GetNpcMine(int id) const
{
	return _GetChdVec(m_NMTab, id);
}

const ChdVec * CNmdChd::GetDynObj(int id) const
{
	return _GetChdVec(m_DynTab, id);
}

}
