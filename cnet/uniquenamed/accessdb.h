#ifndef __UNIQUENAME_GNET_ACCESSDB_H
#define __UNIQUENAME_GNET_ACCESSDB_H

#include "timer.h"
#include "uniquenameserver.hpp"

namespace GNET
{

bool ShowInfo( );
bool SetLogicuidNextid( int nextid );
bool SetFactionNextid( int nextid );
bool SetFamilyNextid( int nextid );

bool QueryUser( int userid );
bool QueryRoleByName( const char * name );
bool QueryFactionByName( const char * name );
bool QueryFamilyByName( const char * name );

bool AddLogicuid( int userid, int logicuid );
bool AddRole( const char * name, int zoneid, int roleid, int status = UNIQUENAME_USED, int time = Timer::GetTime() );
bool AddFaction( const char * name, int zoneid, int fid, int status = UNIQUENAME_USED, int time = Timer::GetTime() );
bool AddFamily( const char * name, int zoneid, int fid, int status = UNIQUENAME_USED, int time = Timer::GetTime() );

void ExportCsvLogicuid( );
void ExportCsvRoleId( );
void ExportCsvRoleName( );
void ExportCsvFaction( );
void ExportCsvFamily( );

bool ImportCsvLogicuid( const char * filename );
bool ImportCsvRole( const char * filename );
bool ImportCsvFaction( const char * filename );
bool ImportCsvFamily( const char * filename );

bool ImportRoleList(unsigned int userid, unsigned int rolelist);

void MergeDB( const char * srcpath, const char * srcdbname, const char * destdbname );
void MergeDBAll( const char * srcpath );

}

#endif

