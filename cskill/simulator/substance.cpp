#include <stdlib.h>
#include "substance.h"

ClassInfo substance::m_classsubstance("substance",-1,NULL,substance::CreateObject);
ClassInfo * substance::GetRunTimeClass(){ return &substance::m_classsubstance;}
substance * substance::CreateObject() { return NULL;}

ClassInfo * glb_info_tail = NULL;
ClassInfo * glb_info_head = NULL;


ClassInfo::ClassInfo (const char * name, int guid,ClassInfo*pBase,substance *(*pfCreate)())
			:_name(name),_guid(guid),_pfCreate(pfCreate),_base(pBase)
{
}

ClassInfo::~ClassInfo ()
{
}

ClassInfo *
ClassInfo::GetRunTimeClass(const char *name)
{
	return NULL;
}

ClassInfo * 
ClassInfo::GetRunTimeClass(int guid)
{
	return NULL;
}

bool 
ClassInfo::IsDerivedFrom(const ClassInfo * base)
{
	return false;
}

