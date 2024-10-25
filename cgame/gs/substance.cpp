#include <stdlib.h>
#include "substance.h"
#include <hashmap.h>
#include "dbgprt.h"

ClassInfo substance::m_classsubstance("substance",-1,NULL,substance::CreateObject,0);
ClassInfo * substance::GetRunTimeClass(){ return &substance::m_classsubstance;}
substance * substance::CreateObject() { return NULL;}
size_t 	substance::GetSubstanceSize() { return sizeof(substance);}\

ClassInfo * glb_info_tail = NULL;
ClassInfo * glb_info_head = NULL;

typedef abase::hash_map<int,ClassInfo *> CLASSINFO_MAP;

static CLASSINFO_MAP & GetClsInfoMap()
{
	static CLASSINFO_MAP _map(1024);
	return _map;
}

ClassInfo::ClassInfo (const char * name, int guid,ClassInfo*pBase,substance *(*pfCreate)(), size_t len)
			:_name(name),_guid(guid),_pfCreate(pfCreate),_base(pBase)
{
//测试唯一性
	ClassInfo * tmp = glb_info_head;
	if(guid != -1){
		//guid是-1的话，代表是抽象类 可以重复
		//只有非抽象类才能用guid查询ClassInfo
		ASSERT(GetClsInfoMap().find(guid) == GetClsInfoMap().end());
		GetClsInfoMap()[guid] = this;
		while(tmp)
		{
			ASSERT(tmp->_guid != guid);
			ASSERT(strcmp(tmp->_name,name));
			tmp = tmp->_next;
		}
	}
	
	_next = NULL;
	_prev = glb_info_tail;

	if(glb_info_tail)
	{
		glb_info_tail->_next  = this;
	}
	else
	{
		glb_info_head = this;
	}
	ASSERT(glb_info_head);
	glb_info_tail = this;

	__PRINTINFO("substance name %s, size %u\n",name, len);
	
}

ClassInfo::~ClassInfo ()
{
	if(_prev)
	{
		_prev->_next = _next;
	}
	if(_next)
	{
		_next->_prev = _prev;
	}
	if(glb_info_head == this)
	{
		glb_info_head = _next;
	}
	if(glb_info_tail == this)
	{
		glb_info_tail = _prev;
	}
	//ASSERT(GetClsInfoMap().find(_guid) != GetClsInfoMap().end());
	GetClsInfoMap().erase(_guid);
}

ClassInfo *
ClassInfo::GetRunTimeClass(const char *name)
{
	ClassInfo * tmp = glb_info_head;
	while(tmp)
	{
		if(strcmp(tmp->_name,name) == 0) return tmp;
		tmp = tmp->_next;
	}
	return NULL;
}

ClassInfo * 
ClassInfo::GetRunTimeClass(int guid)
{
	CLASSINFO_MAP &map = GetClsInfoMap();
	CLASSINFO_MAP::iterator it = map.find(guid);
	if(it == map.end()) return NULL;

	return it->second;
/*	ClassInfo * tmp = glb_info_head;
	while(tmp)
	{
		if(tmp->_guid == guid) return tmp;
		tmp = tmp->_next;
	}
	return NULL;*/
}

bool 
ClassInfo::IsDerivedFrom(const ClassInfo * base)
{
	ClassInfo * tmp = this;
	while(tmp)
	{
		if(tmp == base) //或者strcmp(tmp->GetName(),base->GetName()) == 0
		{
			return true;
		}
		tmp = tmp->GetBaseClass();
	}
	return false;
}

