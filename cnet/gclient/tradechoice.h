#ifndef __GNET_TRADECHOICE_H
#define __GNET_TRADECHOICE_H

#include "tradeaddgoods.hpp"
#include "traderemovegoods.hpp"
#include "tradesubmit.hpp"
#include "tradeconfirm.hpp"
#include "tradediscard.hpp"
#include "trademoveobj.hpp"

#include "glinkclient.hpp"

void TradeChoice(unsigned int tid,int roleid,Protocol::Manager* manager,Protocol::Manager::Session::ID sid)
{
BEGIN:
	printf("Add goods/Reomve goods/Submit trade/Confirm trade/Discard trade/Move object?(a/r/s/c/d/m)"); fflush(stdout);
	char choice[128];
	fgets(choice,20,stdin);
	choice[strlen(choice)-1]='\0';
	if (strcmp(choice,"a")==0) 
	{
		int id,pos,count;
		unsigned int money;
		printf("input id, pos, count,money:");fflush(stdout);
		fgets(choice,127,stdin);
		choice[strlen(choice)-1]='\0';
		if (4!=sscanf(choice,"%d %d %d %d",&id,&pos,&count,&money)) {printf("error input!\n"); return; }
		manager->Send(sid,TradeAddGoods(tid,roleid,0/*localsid*/,GRoleInventory(id,pos,count,0/*max_count*/,0/*container id*/),money));
	}
	else if (strcmp(choice,"r")==0)
	{	
		int id,pos,count;
		unsigned int money;
		printf("input id, pos, count,money:");fflush(stdout);
		fgets(choice,127,stdin);
		choice[strlen(choice)-1]='\0';
		if (4!=sscanf(choice,"%d %d %d %d",&id,&pos,&count,&money)) {printf("error input!\n"); return; }
		manager->Send(sid,TradeRemoveGoods(tid,roleid,0/*localsid*/,GRoleInventory(id,pos,count,3/*max_count*/,1/*container id*/),money));
		printf("send remove goods.\n");

	}
	else if (strcmp(choice,"m")==0)
	{
		int id,pos,count,dstpos;
		printf("input id, pos, count,toPos:");fflush(stdout);
		fgets(choice,127,stdin);
		choice[strlen(choice)-1]='\0';
		if (4!=sscanf(choice,"%d %d %d %d",&id,&pos,&count,&dstpos)) {printf("error input!\n"); return; }
		manager->Send(sid,TradeMoveObj(tid,roleid,0/*localsid*/,GRoleInventory(id,pos,count,3/*max_count*/,1/*container id*/),dstpos));

	}
	else if (strcmp(choice,"s")==0)
	{
		manager->Send(sid,TradeSubmit(tid,roleid,0));
	}
	else if (strcmp(choice,"c")==0)
	{
		manager->Send(sid,TradeConfirm(tid,roleid,0));
	}
	else if (strcmp(choice,"d")==0)
	{
		manager->Send(sid,TradeDiscard(tid,roleid,0));
	}
	else
	{
		printf("invalid input.\n");
		goto BEGIN;
	}
}
#endif
