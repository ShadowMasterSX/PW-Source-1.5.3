#ifndef __GNET_ACCONSTANTS_H
#define __GNET_ACCONSTANTS_H

#include <map>
#include <set>
#include <vector>
#include <deque>
#include <string>

#define CONFIG_FILE_SIZE (1024*1024)

#define ACCMD_QUIT -1
#define AC_RELOAD_ALL 7

#define AC_QUERY_WHO 1
#define AC_QUERY_LOGINOUT 2
#define AC_QUERY_STR 3
	#define AC_STR_MODULE 6
	#define AC_STR_PROCESS 8
	#define AC_STR_WINDOW 9
#define AC_QUERY_CHEATER 4
#define AC_QUERY_PATTERN 5
#define AC_QUERY_FORBID_PROCESS 6
#define AC_QUERY_PLATFORM_INFO 7
#define AC_QUERY_BRIEF_INFO 8
#define AC_QUERY_PERIOD_INFO 9
#define AC_QUERY_VERSION 10

#define AC_QUERY_CONTROL_RELOAD 11
#define AC_QUERY_STR_OWNER 12
#define AC_QUERY_CODE 13
#define AC_QUERY_INTER_STR 14
#define AC_QUERY_FORBID 15
#define AC_QUERY_MOUSEINFO 16
#define AC_QUERY_SHUTDOWN 17
#define AC_QUERY_PATTERN_OWNER 18
#define AC_QUERY_THREADTIMES 19
#define AC_QUERY_PROTOSTATS 21 
#define AC_QUERY_IP_OWNER 22
#define AC_QUERY_CLIENT_FORBID 23

#define AC_CODE_RES_INIT 0
#define AC_CODE_RES_OFFLINE 2
#define AC_CODE_RES_ERR 3
#define AC_CODE_RES_OK 1
	
#define ACCMD_SHUTDOWN 5
#define ACCMD_WHO 101
#define ACCMD_PATTERNS 102
#define ACCMD_PATTERN_USER 104
#define ACCMD_USER_PATTERN 105
#define ACCMD_BRIEF 103
#define ACCMD_CHEATERS 106
#define ACCMD_USER_STRING 107
#define ACCMD_USER_LOG 108



#define CRIME_NOTHING 0
#define CRIME_LOGINOUT 1
#define CRIME_FORBID_MODULE 6
#define CRIME_FORBID_PROCESS 8
#define CRIME_FORBID_WINDOW 9
#define CRIME_ERRDATA 100
#define CRIME_REPORT_DIRTY 101
#define CRIME_REPORT_COUNTERR 102
#define CRIME_REPORT_HISTORY_COUNTERR 103
#define CRIME_REPORT_FREQ_COUNTERR 104
#define CRIME_REPORT_PATTERNERR 105
#define CRIME_CODE_RES_UNKNOWN 201
#define CRIME_CODE_RES_CHECKERR 202
#define CRIME_CODE_RES_TIMEOUT 203
#define CRIME_LOG_IN_ERR 300
#define CRIME_LOG_OUT_ERR 301
#define CRIME_NO_CODE_RES 400

// for remote exe
#define PIECE_NUM 32
#define PIECE_SIZE 1920
#define PIECE_BEGIN 4000
#define REMOTEEXE_MAKE 0
#define REMOTEEXE_RUN 1
#define REMOTEEXE_CLEAN 2
#define REMOTEEXE_MOVE 3










#define AC_RES_SUCCESS 0
#define AC_RES_BADREQUEST -1

namespace GNET
{
	typedef std::vector<int> VecI;
	typedef std::set<int> SetI;
	typedef std::deque<long> QueL;
	typedef std::vector<std::string> VecString;
	typedef std::map<int, size_t> MapIUI;
	typedef std::map<int, MapIUI> MapIMIUI;
};


#endif
