#ifndef __GNET_MAKECHOICE_H
#define __GNET_MAKECHOICE_H

#include "selectrole.hpp"
#include "createrole.hpp"
#include "deleterole.hpp"
#include "undodeleterole.hpp"
#include "../common/conv_charset.h"

#include "glinkclient.hpp"
namespace GNET
{
	
void MakeChoice(int userid,Protocol::Manager* manager,Protocol::Manager::Session::ID sid)
{
BEGIN:	
	printf("Create Role, Delete Role ,Select Role or Undo Delete Role?(c/d/s/u)"); fflush(stdout);
	char choice[128];
	fgets(choice,20,stdin);
	choice[strlen(choice)-1]='\0';
	if (strcmp(choice,"c")==0) 
	{
		int gender,occp;
		char name[32];
		printf("input gender, occupation, name:");fflush(stdout);
		fgets(choice,127,stdin);
		choice[strlen(choice)-1]='\0';
		if (3!=sscanf(choice,"%d %d %s",&gender,&occp,name)) {printf("error input!\n"); return; }
		Octets name_ucs2;
		//conv_charset("GBK","UCS2",Octets(name,strlen(name)),name_ucs2);
		CharsetConverter::conv_charset_g2u( Octets(name,strlen(name)),name_ucs2 );
		manager->Send(sid,CreateRole(userid,_SID_INVALID,RoleInfo(_ROLE_INVALID,gender/*male*/,0/*race*/,occp/*occupation*/,1/*level*/,0,name_ucs2,Octets(0)),Octets(0)));
	}
	else if (strcmp(choice,"s")==0)
	{
		printf("Select:: input roleid(%d-%d):",userid+0,userid+15); fflush(stdout);
		char p[16];
		fgets(p,16,stdin); 
		p[strlen(p)-1]='\0';
		manager->Send(sid,SelectRole(atoi(p)));
		printf("client: send selectrole,roleid=%d\n",atoi(p));
		GLinkClient::GetInstance()->roleid=atoi(p);
	}
	else if (strcmp(choice,"d")==0)
	{
		printf("Delete:: input roleid(%d-%d):",userid+0,userid+15); fflush(stdout);
		char p[16];
		fgets(p,16,stdin); 
		p[strlen(p)-1]='\0';
		manager->Send(sid,DeleteRole(atoi(p),_SID_INVALID));
		printf("client: send deleterole,roleid=%d\n",atoi(p));
	}
	else if (strcmp(choice,"u")==0)
	{
		printf("UndoDelete:: input roleid(%d-%d):",userid+0,userid+15); fflush(stdout);
		char p[16];
		fgets(p,16,stdin); 
		p[strlen(p)-1]='\0';
		manager->Send(sid,UndoDeleteRole(atoi(p),_SID_INVALID));
		printf("client: send undo_deleterole,roleid=%d\n",atoi(p));
	}
	else
	{
		printf("invalid input\n");
		goto BEGIN;
	}
}

};
#endif
