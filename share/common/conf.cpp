
#include "conf.h"

namespace GNET
{

Conf Conf::instance;
Thread::RWLock Conf::locker("Conf::locker");

};

