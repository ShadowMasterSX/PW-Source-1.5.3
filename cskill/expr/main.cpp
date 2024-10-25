#include <sys/time.h>
#include <mcheck.h>
#include <iostream>
#include "expr.h"
using namespace GNET;

int main(int argc,char* argv[])
{
	setenv("MALLOC_TRACE","./mtrace.log",1);
	mtrace();
	if (argc != 2 || access(argv[1],R_OK))
	{
		printf("Usage: compiler <*.og>\n");
		return 0;
	}
	try
	{
			
		/*prepare data*/
		SkillLevel sl;
		PlayerWrapper p;
		Skill s;
		s.SetPlayer(&p);
		SkillCO sco(&s);	
		/**************/
		
		Expr::GetInstance(argv[1]);
		/*
		for (int i=0;i<10;i++)
			Expr::GetInstance().UpdateExpr(std::string("t"), "player.level=5 % 3; player.hp=player.mp!=3 & player.mp!=15  ? 9.3*player.level:1+level; player.mp=(5+2)*3+6.5; player.skilllevel[3]=player.level;" ,"Skill");
			
		Expr::GetInstance().Transact(std::string("t"),&sco);
		printf("player's mp=%d,player's hp=%d,player's level=%d,skilllevel[3]=%d\n\n",p.mp,p.hp,p.level,sl[3]);
		*/
		for (int i=0;i<100;i++)
			Expr::GetInstance().UpdateExpr(std::string("sample.tny"),"Skill");
		
		for (int i=0;i<100;i++)
		{
			Expr::GetInstance().Transact(std::string("sample.tny"),&sco);
			//printf("player's mp=%d,player's hp=%d,player's level=%d,skilllevel[3]=%d\n",p.mp,p.hp,p.level,sl[3]);
		}
		
		//Expr::GetInstance().UpdateExpr(std::string("c"),"(5+2)*5-7;","Skill");
		//Expr::GetInstance().UpdateExpr(std::string("c"),"(5+2)*5;","Skill");
		
		/*
		std::string key("sample.tny");
		struct timeval begin,end;
		gettimeofday(&begin,NULL);	
		for (int i=0;i<100000;i++)
		{
			Expr::GetInstance().Transact(key,&sco);
		}
		gettimeofday(&end,NULL);	
		printf("Executor time: %d s, %d ms\n",(end.tv_usec-begin.tv_usec)>=0 ? end.tv_sec-begin.tv_sec:end.tv_sec-begin.tv_sec-1,(end.tv_usec-begin.tv_usec)>=0 ? (end.tv_usec-begin.tv_usec)/1000 : 1000+(end.tv_usec-begin.tv_usec)/1000);
		
		gettimeofday(&begin,NULL);	
		for (int i=0;i<100000;i++)
		{
			p.level=5 % 3;
			p.hp=p.mp>=2 && p.mp<=15 ? 0.3*p.level:1+s.level;
			p.mp=(5+2)*3+6.5;
			p.level=p.skilllevel[5];
		}
		gettimeofday(&end,NULL);	
		printf("C++ Executor time: %d s, %d ms\n",(end.tv_usec-begin.tv_usec)>=0 ? end.tv_sec-begin.tv_sec:end.tv_sec-begin.tv_sec-1,(end.tv_usec-begin.tv_usec)>=0 ? (end.tv_usec-begin.tv_usec)/1000 : 1000+(end.tv_usec-begin.tv_usec)/1000);
		*/
	}
	catch (...)
	{
		printf("catch exception in main.\n");
		return 0;
	}
	muntrace();
}
