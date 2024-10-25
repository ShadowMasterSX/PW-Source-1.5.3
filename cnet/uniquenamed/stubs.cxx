#ifdef WIN32
#include <winsock2.h>
#include "gncompress.h"
#else
#include "binder.h"
#endif
#include "precreaterole.hrp"
#include "postcreaterole.hrp"
#include "postdeleterole.hrp"
#include "precreatefaction.hrp"
#include "postcreatefaction.hrp"
#include "postdeletefaction.hrp"
#include "rolenameexists.hrp"
#include "userrolecount.hrp"
#include "moverolecreate.hrp"
#include "precreatefamily.hrp"
#include "postcreatefamily.hrp"
#include "postdeletefamily.hrp"
#include "dbrawread.hrp"
#include "preplayerrename.hrp"
#include "prefactionrename.hrp"
#include "keepalive.hpp"
#include "postplayerrename.hpp"
#include "postfactionrename.hpp"

namespace GNET
{

static PreCreateRole __stub_PreCreateRole (RPC_PRECREATEROLE, new PreCreateRoleArg, new PreCreateRoleRes);
static PostCreateRole __stub_PostCreateRole (RPC_POSTCREATEROLE, new PostCreateRoleArg, new PostCreateRoleRes);
static PostDeleteRole __stub_PostDeleteRole (RPC_POSTDELETEROLE, new PostDeleteRoleArg, new PostDeleteRoleRes);
static PreCreateFaction __stub_PreCreateFaction (RPC_PRECREATEFACTION, new PreCreateFactionArg, new PreCreateFactionRes);
static PostCreateFaction __stub_PostCreateFaction (RPC_POSTCREATEFACTION, new PostCreateFactionArg, new PostCreateFactionRes);
static PostDeleteFaction __stub_PostDeleteFaction (RPC_POSTDELETEFACTION, new PostDeleteFactionArg, new PostDeleteFactionRes);
static RolenameExists __stub_RolenameExists (RPC_ROLENAMEEXISTS, new RolenameExistsArg, new RolenameExistsRes);
static UserRoleCount __stub_UserRoleCount (RPC_USERROLECOUNT, new UserRoleCountArg, new UserRoleCountRes);
static MoveRoleCreate __stub_MoveRoleCreate (RPC_MOVEROLECREATE, new MoveRoleCreateArg, new MoveRoleCreateRes);
static PreCreateFamily __stub_PreCreateFamily (RPC_PRECREATEFAMILY, new PreCreateFamilyArg, new PreCreateFamilyRes);
static PostCreateFamily __stub_PostCreateFamily (RPC_POSTCREATEFAMILY, new PostCreateFamilyArg, new PostCreateFamilyRes);
static PostDeleteFamily __stub_PostDeleteFamily (RPC_POSTDELETEFAMILY, new PostDeleteFamilyArg, new PostDeleteFamilyRes);
static DBRawRead __stub_DBRawRead (RPC_DBRAWREAD, new DBRawReadArg, new DBRawReadRes);
static PrePlayerRename __stub_PrePlayerRename (RPC_PREPLAYERRENAME, new PrePlayerRenameArg, new PrePlayerRenameRes);
static PreFactionRename __stub_PreFactionRename (RPC_PREFACTIONRENAME, new PreFactionRenameArg, new PreFactionRenameRes);
static KeepAlive __stub_KeepAlive((void*)0);
static PostPlayerRename __stub_PostPlayerRename((void*)0);
static PostFactionRename __stub_PostFactionRename((void*)0);

};
