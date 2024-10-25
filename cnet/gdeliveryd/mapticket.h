#ifndef __GNET_MAPTICKET_H
#define __GNET_MAPTICKET_H

#include <map>

#include "thread.h"

namespace GNET
{

class FaceTicketSet
{
public:
	struct face_ticket_t
	{
		int id;
		int pos;
		face_ticket_t(int _id=-1,int _pos=-1) : id(_id),pos(_pos) { }
	};
private:
	typedef std::map<int/*roleid*/, face_ticket_t> Map;
	Map	map;
	Thread::Mutex locker;
	static FaceTicketSet	instance;
public:
	FaceTicketSet() : locker("FaceTicketSet::locker") { }
	~FaceTicketSet() { }
	static FaceTicketSet & GetInstance() { return instance; }
	size_t Size() { Thread::Mutex::Scoped l(locker);	return map.size();	}

	void AddTicket(int roleid,const face_ticket_t& t) 
	{
		Thread::Mutex::Scoped l(locker);
		map[roleid]=t; 
	}

	void RemoveTicket(int roleid)
	{
		Thread::Mutex::Scoped l(locker);
		map.erase(roleid);
	}

	bool FindTicket(int roleid, face_ticket_t& t)
	{
		Thread::Mutex::Scoped l(locker);
		std::map<int/*roleid*/,face_ticket_t>::iterator it=map.find(roleid);
		if ( it==map.end() )
			return false;
		else
		{
			t=(*it).second;
			return true;
		}
	}

	bool HasTicket(int roleid)
	{
		Thread::Mutex::Scoped l(locker);
		return map.find(roleid)!=map.end();
	}
};

};

#endif

