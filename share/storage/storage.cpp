
#include "storage.h"

#if defined USE_BDB
#include "storagebdb.h"
namespace BDB
{
DbEnv * BDB::StorageEnv::env;
BDB::StorageEnv::StorageMap BDB::StorageEnv::smap;
Thread::Mutex BDB::StorageEnv::locker_smap("StorageEnv::locker_smap");
std::string BDB::StorageEnv::homedir;
std::string BDB::StorageEnv::datadir;
std::string BDB::StorageEnv::datadirorg;
std::string BDB::StorageEnv::logdir;
std::string BDB::StorageEnv::logdirorg;
bool BDB::StorageEnv::iscompress;
#if defined _REENTRANT
BDB::StorageEnv::ThreadTransaction::Map BDB::StorageEnv::ThreadTransaction::map;
Thread::Mutex BDB::StorageEnv::ThreadTransaction::locker_map("StorageEnv::ThreadTransaction::locker_map");
BDB::StorageEnv::TransactionRunnable BDB::StorageEnv::ThreadTransaction::transaction_null;
#endif
};
#endif

#if defined USE_CDB
#include "storagecdb.h"
namespace CDB
{
DbEnv * CDB::StorageEnv::env;
CDB::StorageEnv::StorageMap CDB::StorageEnv::smap;
Thread::Mutex CDB::StorageEnv::locker_smap("StorageEnv::locker_smap");
std::string CDB::StorageEnv::homedir;
std::string CDB::StorageEnv::datadir;
std::string CDB::StorageEnv::datadirorg;
std::string CDB::StorageEnv::logdir;
std::string CDB::StorageEnv::logdirorg;
bool CDB::StorageEnv::iscompress;
};
#endif

#if defined USE_WDB
#include "storagewdb.h"
namespace WDB
{
WDB::StorageEnv::StorageMap WDB::StorageEnv::storage_map;
WDB::StorageEnv::StorageVec WDB::StorageEnv::storage_vec;
GNET::DBCollection *WDB::StorageEnv::env;
pthread_key_t WDB::StorageEnv::ThreadContext::key;
//Thread::Mutex WDB::StorageEnv::locker_smap("StorageEnv::locker_smap");
std::string WDB::StorageEnv::homedir;
std::string WDB::StorageEnv::datadir;
std::string WDB::StorageEnv::logdir;
//bool WDB::StorageEnv::iscompress;
GNET::Thread::Mutex WDB::StorageEnv::checkpoint_locker;
};
#endif


