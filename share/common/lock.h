#ifndef __LOCK_H
#define __LOCK_H

#include <map>
#include <vector>
#include "mutex.h"

namespace GNET
{

class LockTable;
class Role
{
	LockTable *lock_table;
public:
	virtual ~Role() { UnlockAll(); }
	Role(LockTable * ltable) : lock_table(ltable) { }
	int Lock( void *resource );
	void Unlock( void *resource ); 
	void UnlockAll();
	virtual void Notify( void *resource ) { printf("%p notified %p\n", this, resource); } 
};

class LockTable
{
	typedef std::map<Role *, void *> PRQ;
	typedef std::map<void *, Role *> GRQ;
	PRQ	prq;
	GRQ	grq;
	Thread::Mutex locker("LockTable");
public:
	LockTable() : locker(true) { }
	enum { GRANT, BLOCK, DEADLOCK };
	int Lock( Role *role, void *resource )
	{
		Thread::Mutex::Scoped l(locker);
		PRQ::iterator it_prq = prq.find(role);
		if ( it_prq != prq.end() )
			return BLOCK;
		
		GRQ::iterator it_grq = grq.find(resource);
		if ( it_grq == grq.end() )
		{
			grq.insert( std::make_pair( resource, role ) );
			return GRANT;
		}
		
		Role *r = (*it_grq).second;
		if ( r == role )
			return GRANT;

		prq.insert( std::make_pair( role, resource ) );
		while ( (it_prq = prq.find(r)) != prq.end() && (it_grq = grq.find((*it_prq).second)) != grq.end() )
			if ( (r = (*it_grq).second) == role )
				return DEADLOCK;

		return BLOCK;
	}

	void Unlock( Role *role, void *resource )
	{
		std::vector<Role *> r;

		{
		Thread::Mutex::Scoped l(locker);
		PRQ::iterator it_prq = prq.find(role);
		if ( it_prq != prq.end() && (*it_prq).second == resource )
		{
			prq.erase(it_prq);
			return;
		}

		GRQ::iterator it_grq = grq.find(resource);
		if ( it_grq == grq.end() || (*it_grq).second != role )
			return;

		grq.erase(it_grq);

		for (PRQ::iterator it = prq.begin(), ie = prq.end(); it != ie; ++it )
			if ( (*it).second == resource )
				r.push_back( (*it).first );
		}

		for (std::vector<Role *>::iterator it = r.begin(), ie = r.end(); it != ie; ++it )
			(*it)->Notify( resource );
	}

	void Unlock( Role *role )
	{
		std::vector<std::pair<Role*, void*> > r;
		{
		Thread::Mutex::Scoped l(locker);
		PRQ::iterator it_prq = prq.find(role);
		if ( it_prq != prq.end() )
			prq.erase(it_prq);

		std::vector<GRQ::iterator> it_grq_vec;

		for (GRQ::iterator it = grq.begin(), ie = grq.end(); it != ie; ++it )
			if ( (*it).second == role )
				it_grq_vec.push_back(it);

		PRQ::iterator iep = prq.end();
		for (std::vector<GRQ::iterator>::iterator itg = it_grq_vec.begin(), ieg = it_grq_vec.end(); itg != ieg; ++itg )
		{
			void *resource = (**itg).first;
			for (PRQ::iterator itp = prq.begin(); itp != iep; ++itp )
				if ( (*itp).second == resource )
					r.push_back ( std::make_pair( (*itp).first, resource ) );
			grq.erase(*itg);
		}
		}

		for (std::vector<std::pair<Role*, void*> >::iterator it = r.begin(), ie = r.end(); it != ie; ++it )
			(*it).first->Notify( (*it).second );
	}
};

inline int  Role::Lock( void *resource ) { return lock_table->Lock( this, resource ); }
inline void Role::Unlock( void *resource ) { lock_table->Unlock( this, resource ); }
inline void Role::UnlockAll() { lock_table->Unlock(this); }

};

#endif
