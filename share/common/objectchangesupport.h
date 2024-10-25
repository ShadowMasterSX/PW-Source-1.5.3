#ifndef __GNET_OBJECTCHANGESUPPORT_HPP
#define __GNET_OBJECTCHANGESUPPORT_HPP

#include "objectchangelistener.h"
#include "thread.h"

#include <vector>
#include <algorithm>
#include <functional>

namespace GNET
{

class ObjectChangeSupport
{
	Thread::Mutex locker;
	std::vector<ObjectChangeListener *> container;
public:
	virtual ~ObjectChangeSupport() { }
	void AddObjectChangeListener( ObjectChangeListener *l )
	{
		Thread::Mutex::Scoped lk(locker);
		container.push_back(l);
	}
	void RemoteObjectChangeListener( ObjectChangeListener *l )
	{
		Thread::Mutex::Scoped lk(locker);
		container.erase( std::remove(container.begin(), container.end(), l), container.end() );
	}
	void FireObjectChange()
	{
		Thread::Mutex::Scoped lk(locker);
#if defined _REENTRANT_
		std::vector<ObjectChangeListener *> c(container);
#else
		std::vector<ObjectChangeListener *> &c = container;
#endif
		std::for_each( c.begin(), c.end(), std::bind2nd(std::mem_fun(&ObjectChangeListener::ObjectChange), this) );
	}
};

};
#endif
