/* This file defines the privilege of a occupation in a faction
 *  
 * @author: liping
 * @date: 2005-4-14
 */ 

#ifndef __GNET_PRIVILEGE_H
#define __GNET_PRIVILEGE_H
#include "ids.hxx"
#include <set>
#include <map>
#include <stddef.h>
namespace GNET
{
	
class Privilege
{
private:
	GNET::Roles faction_role;
	std::set<GNET::Operations> op_allowed;

	typedef std::map<GNET::Roles,Privilege*> StubMap;
	static StubMap& GetMap()
	{
		static StubMap stubmap;
		return stubmap;
	}
public:
	Privilege(GNET::Roles _role,GNET::Operations *_oplist,size_t size) : faction_role(_role),op_allowed(_oplist,_oplist+size) 
	{
		if (GetStub(_role)==NULL) 
		{
			GetMap().insert( std::make_pair(_role,this) );
		}
	}
	static Privilege* GetStub(GNET::Roles _role) 
	{
		StubMap::iterator it=GetMap().find(_role);
		//printf("map size is %d\n",GetMap().size());
		return it==GetMap().end() ? NULL:(*it).second;
	}
	bool PrivilegeCheck(GNET::Operations op) 
	{
		return op_allowed.find(op) != op_allowed.end();
	}
};

};
#endif
