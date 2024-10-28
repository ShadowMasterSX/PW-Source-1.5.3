#ifndef __GNET_ACCESSDB_H
#define __GNET_ACCESSDB_H

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "log.h"
#include "dbbuffer.h"

#include "stocklog"
#include "user"
#include "roleid"
#include "groleforbid"
#include "grolebase"
#include "grolestatus"
#include "groleinventory"
#include "grolestorehouse"
#include "gshoplog"
#include "groledetail"
#include "gfactioninfo"
#include "guserfaction"

#include "gamedbmanager.h"
#include "timer.h"

#define DB_CONFIG2_KEY 101
#define CHECKPOINT_THRESHOLD 1024

namespace GNET
{
void PrintLogicuid( );
void PrintUnamerole( int zoneid );
void PrintUnamefaction( int zoneid );
void GenNameIdx( );
void ExportUnique( int zoneid );

bool QueryRole( int roleid );
bool ExportRole( int roleid );
bool ClearRole( int roleid, char * tablename );
bool ExportUser( int userid );
bool ModifyCash( int userid, int cash_add, int cash_used );

void ListRole( );
void ListRoleBrief(std::string& login_time_str);
void ListUserBrief( );
void ListFaction( );
void ListFactionUser( );
void ListFactionRelation();
void ListCity( );
void ListShopLog();
void ListSysLog();
void ListRoleInventory();

void UpdateRoles( );
void ConvertDB( );
void RepairDB( );

void MergeDB( const char * srcpath, const char * srcdbname, const char * destdbname );
void MergeDBAll( const char * srcpath );

void GetTimeoutRole( IntVector& rolelist );
void SetCashInvisible(const char* file);

void ExportRoleList(const char *roleidfile, const char *rolelistfile);
void ImportRoleList(const char *rolelistfiles[], int filecount);

void WalkAllTable();
void ExportGameTalkData(const char *outputdir);
void ExportUserStore(int roleid);
void ExportProfitTime();

void SyncPlayerName();

void ReturnCash(const char* useridfile);
void ImportRecallUser(const char* useridfile);

void QueryWebTrade(int64_t sn);

void QueryGlobalControl();
//cross server related
int GetDBCrossType();
void SetDBCrossType(bool is_central);
int GetStandaloneDBCrossType(const char* srcpath);
void AbstractPlayers(const char* srcpath, int zoneid);
void DelCentralZonePlayers(int zoneid);
void ListRoleCrossInfo(int roleid);
void ResetRoleCrossInfo(int roleid, int remote_roleid, int data_timestamp, int cross_timestamp, int src_zoneid);

void ImportRole(const char* roleidfile);
bool DeleteWaitdel(bool backup);
void CalcWaitdel(int year,int level);
void GetSigninData(int month);
void SetClsPVPFlag(int flag);
void GetRoleLoginData(int year, int logicid);
void GetUserLoginData(int year, int logicid);
void ListFinTaskCount(int count);

void ListMNFactionInfo();
void ListMNFactionApplyInfo();
void ListMNDomainInfo();
void ClearMNFactionId();

void ImportMNFactionInfo(const char* mnfactioninfofile);
void ImportMNFactionApplyInfo(const char* mnfactionapplyinfofile);
void ImportMNDomainInfo(const char* mndomaininfofile);

void SetMNFactionState(int state);
void ClearMNFactionCredit();
void ListSoloChallengeRank();
void ClearSoloChallengeRank(int roleid, int zoneid);

}

namespace CMGDB
{
void MergeDB( const char * srcpath, const char * srcdbname, const char * destdbname );
void MergeDBAll( const char * srcpath, int srczoneid);
}

#endif

