#ifndef __GNET_OBJECTCHANGELISTENER_HPP
#define __GNET_OBJECTCHANGELISTENER_HPP

namespace GNET
{

class ObjectChangeSupport;
class ObjectChangeListener
{
public:
	virtual void ObjectChange(const ObjectChangeSupport *) = 0;
};

};
#endif
