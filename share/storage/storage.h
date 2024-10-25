#ifndef __STORAGE_H
#define __STORAGE_H

#define STORAGE_CONFIGDB	"config"

#if defined USE_BDB
#include "storagebdb.h"
using namespace BDB;	// Berkley db4
#elif defined USE_CDB
#include "storagecdb.h"
using namespace CDB;	// Berkley db4 which cache
#elif defined USE_WDB
#include "storagewdb.h"
using namespace WDB;	// World2 db
#endif

#endif
