
#include "commander.hpp"
#include "ganticheaterclient.hpp"
#include "acreloadconfig.hpp"
#include "acquery.hpp"
#include "acsendcode.hpp"
#include <iostream>

namespace GNET
{

Commander Commander::instance;

void Commander::ParseCommandLine(std::string line)
{
	//std::string line;
	std::vector<std::string> cmds;
//	bool inputok = false;
//	bool first =true;
	//std::string qid;
	//std::string param;
	//size_t pn = 0;
	//size_t i = 0;
	//while( !inputok )
	//{
	//	if( !first )
	//	{
	//		printf("command not found\n");
	//		cmds.clear();
	//	}
	//	else
	//		first = false;
	//	printf("\n:)");
	//	std::getline(std::cin, line);
	for(size_t jj = 0; jj <1; ++jj)
	{
		if( line == "." )	
		{
			line = cmdcache;
			std::cout<<line<<std::endl;
		}
		else
			cmdcache = line;
		split(line, cmds);

		if( cmds.empty() ) continue;
		std::string cmd = cmds[0];

		if( cmd == "reload" || cmd == "reload_stat"
				|| cmd == "reload_log" || cmd == "reload_code" )
		{
			char buffer[CONFIG_FILE_SIZE];
			FILE *f = fopen("gacd.xml", "r");
			size_t size = fread(buffer, 1, CONFIG_FILE_SIZE, f);
			fclose(f);
			ACReloadConfig acrc;
			acrc.config.replace(buffer, size);
			acrc.reload_type = AC_RELOAD_ALL;
			if( cmd == "reload_stat" ) acrc.reload_type = AC_RELOAD_STAT;
			else if( cmd == "reload_log" ) acrc.reload_type = AC_RELOAD_LOG;
			else if( cmd == "reload_code" ) acrc.reload_type = AC_RELOAD_CODE;
			int t_aid = 0;
			if( cmds.size() > 1 )
				t_aid = atoi(cmds[1].c_str());
			GAntiCheaterClient::SendProtocol( acrc, t_aid );
			return;
		}
		else if( ( cmd == "logs" || cmd == "strs" ) && cmds.size() > 2 )
		{
			ACQuery acql;
			acql.query_type = (cmd == "logs" ? AC_QUERY_LOGINOUT : AC_QUERY_STR );
			acql.roleid = atoi(cmds[2].c_str());
			GAntiCheaterClient::SendProtocol( acql, atoi(cmds[1].c_str()));
			return;
		}
		else if( cmd == "who" && cmds.size() > 1 )
		{
			ACQuery acql;
			acql.query_type = AC_QUERY_WHO;
			GAntiCheaterClient::SendProtocol( acql, atoi(cmds[1].c_str()));
			return;
		}
		else if( cmd == "forbidprocess" && cmds.size() > 1 )
		{
			ACQuery acql;
			acql.query_type = AC_QUERY_FORBID_PROCESS;
			GAntiCheaterClient::SendProtocol( acql, atoi(cmds[1].c_str()));
			return;
		}
		else if( cmd == "patternbrief" && cmds.size() > 1 )
		{
			ACQuery acql;
			acql.query_type = AC_QUERY_BRIEF_INFO;
			GAntiCheaterClient::SendProtocol( acql, atoi(cmds[1].c_str()));
			return;
		}
		else if( cmd == "cheaters" )
		{
			QueryCheaters();
			return;
		}
		else if( cmd == "patterns" )
		{
			ACQuery acql;
            acql.query_type = AC_QUERY_PATTERN;
            acql.roleid = -1; 
            int t_aid = 0;
            if( cmds.size() > 1 ) t_aid = atoi(cmds[1].c_str());
            if( cmds.size() > 2 ) acql.roleid = atoi(cmds[2].c_str());
            GAntiCheaterClient::SendProtocol( acql, t_aid);
            return;
		}
		else if( cmd == "periods" )
		{
			ACQuery acql;
            acql.query_type = AC_QUERY_PERIOD_INFO;
            acql.roleid = -1; 
            int t_aid = 0;
            if( cmds.size() > 1 ) t_aid = atoi(cmds[1].c_str());
            if( cmds.size() > 2 ) acql.roleid = atoi(cmds[2].c_str());
            GAntiCheaterClient::SendProtocol( acql, t_aid);
            return;
		}
		else if( cmd == "sendcode" && cmds.size() > 2 )
		{
			ACSendCode acsc;
			acsc.code_name = cmds[1];
			acsc.param = cmds[2];
			int t_aid = 0;
			if( cmds.size() > 3 ) t_aid = atoi(cmds[3].c_str());
			acsc.roleid = cmds.size() < 5 ? -1 : atoi(cmds[4].c_str());
			GAntiCheaterClient::SendProtocol( acsc, t_aid);
			return;
		}
		else if( cmd == "platforminfo" )
		{
			ACQuery acql;
			acql.query_type = AC_QUERY_PLATFORM_INFO;
			acql.roleid = -1;
			int t_aid = 0;
			if( cmds.size() > 1 ) t_aid = atoi(cmds[1].c_str());
			if( cmds.size() > 2 ) acql.roleid = atoi(cmds[2].c_str());
			GAntiCheaterClient::SendProtocol( acql, t_aid);
			return;
		}
		else if( cmd == "quit" || cmd == "exit" )
		{
			exit(0);
		}
		else if( cmd == "help" )
		{
			printf("\nAll Commands:\n");
			printf("\thelp\n");
			printf("\treload | relod_stat | reload_log | reload_code [ AREA_ID ]\n");
			printf("\tlogs AREA_ID ROLE_ID\n");
			printf("\tstrs AREA_ID ROLE_ID\n");
			printf("\twho AREA_ID\n");
			printf("\tcheaters\n");
			printf("\tpatterns [AREA_ID [ROLE_ID]]\n");
			printf("\tperiods [AREA_ID [ROLE_ID]]\n");
			printf("\tsendcode CODE_NAME PARAM [AREA_ID [ROLE_ID]]\n");
			printf("\tplatforminfo [AREA_ID [ROLE_ID]]\n");
			printf("\tforbidprocess AREA_ID\n");
			printf("\tpatternbrief AREA_ID\n");
			printf("\tquit | exit\n\n");
			return;
		}

		/*

		CmdsMap::const_iterator it = cmdsmap.find(cmds[0]);
		if( it == cmdsmap.end() ) continue;
		int cmd = (*it).second;
		ACData pro;
		//pro.cmd = cmd;
		//printf("cmd %d\n", cmd);
	
		Marshal::OctetsStream os;

		switch( cmd )
		{
			case ACCMD_PATTERNS :
			case ACCMD_BRIEF :
			case ACCMD_CHEATERS :
				break;
			case ACCMD_PATTERN_USER :
				{
					int tpat;
					sscanf(cmds[1].c_str(), "%x", &tpat);
					os << tpat;
					pro.data = os;
				}
				break;
			case ACCMD_USER_PATTERN :
			default :
				break;
		}
		
		if( false == GAntiCheaterClient::GetInstance()->SendProtocol(pro))
			continue;
		return;
		*/
	}
	printf("command not found\n");
	timeout = 1;
}

void Commander::QueryCheaters()
{   
    ACQuery query;
    query.query_type = AC_QUERY_CHEATER;
    GAntiCheaterClient::SendProtocol(query, 0);
} 

void Commander::QueryPatterns()
{
	ACQuery query;
	query.query_type = AC_QUERY_PATTERN;
    GAntiCheaterClient::SendProtocol(query, 0);
}

void Commander::TimerUpdate()
{
	static int tick = 0;
	if( ++tick %4 == 0 )
	{
		if( tick%60 == 0 )
		{
			QueryCheaters();
			//ParseCommandLine("sendcode testparent ` 1");
		}
		if( share_p && *share_p == 1023 )
		{
			
			ACQuery query;
			query.query_type = AC_QUERY_VERSION;
			GAntiCheaterClient::SendProtocol(query, 0);
			*share_p = 116;
			printf("haha\n");
		}
		//ParseCommandLine("sendcode testparent ` 1 673");
		//ParseCommandLine("sendcode testparent ` 1 1984");
	}
	/*
	//if( timeout != 0 )
	{
		if( ++timeout > DEFAULT_TIMEOUT )
		{
			//printf("timeout\n");
			timeout = 0;
			if( !parsing )
			{ parsing = true;
			ParseCommandLine();
				parsing = false;
			}
		}
	}
	*/
}
std::string Commander::replace(const std::string &str, char c1, char c2) const
{
	std::string new_str = str;
	std::string::size_type i;
	while( ( i = new_str.find(c1)) != std::string::npos )
		new_str[i] = c2;
	return new_str;
}
void Commander::split(const std::string &str, std::vector<std::string> &cmds) const
{
    cmds.clear();
    std::string::size_type start = 0;
    std::string::size_type end = 0;
    std::string::size_type endall = str.find_last_not_of(" \t\r\n");
    if( endall == std::string::npos ) return;
    for(;;)
    {
        start = str.find_first_not_of(" \t", end);
        if( start == std::string::npos ) return;
        end = str.find_first_of(" \t", start);
        if( end == std::string::npos )
        {
            cmds.push_back(replace(std::string(str, start, endall - start + 1),'`' ,' '));
            return;
        }
        else
        {
            cmds.push_back(replace(std::string(str, start, end - start), '`', ' '));
        }
    }
	
}

/*

void Commander::ParseResponse(int rescode, int cmd, const Octets &res) 
{
	timeout = 0;
	if( rescode != ACCMD_RES_SUCCESS )
	{
		if( rescode == ACCMD_RES_BADREQUEST )
		{
			printf("bad request!\n");
		}
		//ParseCommandLine();
		return;
	}

	switch( cmd )
	{
		case ACCMD_PATTERNS :
			DumpMapIUI(res, "all pattern", "pattern");
			break;
		case ACCMD_BRIEF :
			DumpBrief(res);
			break;
		case ACCMD_PATTERN_USER :
			DumpMapIUI(res, "all user", "pattern", "%d");
			break;
		case ACCMD_CHEATERS :
			DumpCheaters(res);
			break;
		case ACCMD_USER_PATTERN :
			DumpMapIUI(res, "all ", "pattern");
			break;
		default :
			break;
	}
	printf("\n");
	//ParseCommandLine();
}

void Commander::DumpCheaters(const Octets &res) const
{
	try
	{
		Marshal::OctetsStream os(res);
		size_t n,n2;
		os >> n;
		printf("count %d\n", n);
		if( n > 0 )	
		{
			std::string name;
			int uid;
			time_t tid;
			std::string tstr;
			for(size_t i=0; i<n; ++i)
			{
				os >> name;
				printf("[[%s]]\n", name.c_str());
				os >> n2;
				for(size_t j=0; j<n2; ++j)
				{
					os >> uid;
					os >> tstr;
					os >> tid;
					printf("%d for( %s ), at %s", uid, tstr.c_str(), ctime(&tid));
				}
				printf("\n");
			}
		}
	}
	catch( ... )
	{
		printf("res err\n");
	}	
}
void Commander::DumpBrief(const Octets &res) const
{
	try
	{
		Marshal::OctetsStream os(res);
		printf("%d\n", res.size());
		size_t n;
		os >> n;
		printf("count %d\n", n);
		if( n > 0 )	
		{
			for(size_t i=0; i<n; ++i)
			{
				int bp,caller,p;
				size_t size;
				os >> bp;
				os >> caller;
				os >> size;
				os >> p;
				printf("\tbrief %x <== caller %x, size %d, pattern %x\n",
					bp, caller, size, p);
			}
		}
	}
	catch( ... )
	{
		printf("res err\n");
	}	
}

void Commander::DumpMapIUI(const Octets &res, const char *s1, const char* s2, const char* p)
{
	try
	{
		Marshal::OctetsStream os(res);
		size_t n;
		os >> n;
		typedef std::map<size_t, std::vector<int>, std::greater<size_t> > TmpMap;
		TmpMap tmpmap;
		size_t total = 0;
		if( n > 0 )
		{
			int linkid;
			size_t n2;
			for(size_t i = 0; i<n; ++i)
			{
				os >> linkid;
				os >> n2;
				total += n2;
				tmpmap[n2].push_back(linkid);
			}
			for(TmpMap::const_iterator it = tmpmap.begin(); it != tmpmap.end(); ++it)
			{
				printf(" %s count %d: ", s2, (*it).first);
				printf("\t[ ");
				for(std::vector<int>::const_iterator it2 = (*it).second.begin(); it2 != (*it).second.end();
					++it2)
				{
					printf(p, (*it2));
					printf(", ");
				}
				printf(" ]\n");
			}
		}
		printf("\n%s, %d, %d\n", s1, n, total);
	}
	catch( ... )
	{
		printf("res err\n");
	}	
}

*/

};
