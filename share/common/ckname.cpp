#include <stdio.h>
#include <pcre.h>
#include <iconv.h>
#include "matcher.h"
#include "benchmark.h"

using namespace GNET;
int main(int argc, char** argv)
{
	if(argc<2)
	{
		printf("Usage: ckname <name>\n");
		exit(0);
	}
	Matcher::GetInstance()->Load("filters", "UCS2", "", "GBK");
	
	if (access(argv[1], R_OK) == 0)
	{
		std::ifstream ifs(argv[1]);
		std::string line;

		while(std::getline(ifs, line))
		{
			char* tmp = (char*)line.c_str();
			printf("%d: %s\n", Matcher::GetInstance()->Match(tmp, line.size()), tmp);
		}
	}else
	{
		printf("%d: %s\n", Matcher::GetInstance()->Match(argv[1], strlen(argv[1])), argv[1]);
	}
	//Benchmark bm;
	//bm.tickbegin();
	//for(int i=0;i<1000;i++)
	//	p->Match(argv[1], strlen(argv[1]));
	//bm.tickend();
	//bm.output();

	return 0;
}

