/*
#include "protocol.h"
#include "compress.h"
*/
#include "binder.h"
namespace GNET
{
Protocol::Manager::Session::ID Protocol::Manager::Session::session_id = 0;
Thread::Mutex Protocol::Manager::Session::session_id_mutex("Protocol::Manager::Session::session_id_mutex");
Protocol::Represent::Map* Protocol::Represent::map = new Protocol::Represent::Map();
Thread::Mutex Protocol::Represent::locker("Protocol::Represent::locker");
};
