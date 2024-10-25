#ifndef __GNET_STORAGETOOL_H
#define __GNET_STORAGETOOL_H

#include <sys/types.h>
#include <dirent.h>

#include <set>
#include <map>
#include <vector>
#include <fstream>
#include <string>

#include "storage.h"
#include "log.h"

namespace GNET
{

bool EscapeCSVString( Octets & src, Octets & dest );
bool UnescapeCSVString( Octets & src, Octets & dest );

void TableStat( );
void TableStatRaw( );
void FindMaxsizeValue( const char *dumpfile );

void ReadTable( const char * tablename, int key );
void RewriteTable( const char * fromname, const char * toname );
void RewriteDB( );
void ListId( const char * tablename );
void RewriteTable( const char * keyidfile, const char * fromname, const char * toname );

void CompressDB( const char * dbname );
void CompressDB( );

void DecompressDB( const char * dbname );
void DecompressDB( );

}

#endif

