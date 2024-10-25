
#include "proxyrpc.h"

namespace GNET
{
ProxyRpc::Map ProxyRpc::map;
Thread::RWLock ProxyRpc::locker_map("ProxyRpc::locker_map");
unsigned int ProxyRpc::XID::xid_count = 0;
Thread::Mutex ProxyRpc::XID::locker_xid("ProxyRpc::XID::locker_xid");
ProxyRpc::HouseKeeper ProxyRpc::housekeeper;
};

