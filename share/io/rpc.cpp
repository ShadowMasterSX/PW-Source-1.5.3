
#include "rpc.h"

namespace GNET
{
Rpc::Map Rpc::map;
Thread::RWLock Rpc::locker_map("Rpc::locker_map");
unsigned int Rpc::XID::xid_count = 0;
Thread::Mutex Rpc::XID::locker_xid("Rpc::XID::locker_xid");
Rpc::HouseKeeper Rpc::housekeeper;
};

