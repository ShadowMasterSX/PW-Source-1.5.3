#ifndef __GNET_FACTION_CHOICE_H
#define __GNET_FACTION_CHOICE_H
#include "ids.hxx"
#include <stdio.h>
#include "factionoprequest.hpp"
#include "factionchat.hpp"
#include "factiondata.hxx"
#include "factionacceptjoin.hpp"

#include "factionlib.h"
#include "conv_charset.h"
#include "localmacro.h"
namespace GNET
{
void FactionChoice(int roleid,Protocol::Manager* manager,Protocol::Manager::Session::ID sid)
{
BEGIN:	
	char inputbuf[256];
	printf("OP: create/dismiss/nickname/accept/list/expel/chat/leave/resign/master resign/appoint?(n/d/m/a/l/e/c/v/r/s/p)");
	fflush(stdout);
	fgets(inputbuf,100,stdin);
	inputbuf[strlen(inputbuf)-1]='\0';
	if (inputbuf[0]=='n')
	{
		char choice;
		char fcname[64];
		if (2!=sscanf(inputbuf,"%c %32s",&choice,fcname))
		{
			printf("\ncreate params error. Usage: n <faction name>\n");
			goto BEGIN;
		}
		printf("Try to create faction %s\n",fcname);
		//manager->Send( sid,FactionOPRequest(_O_FACTION_CREATE,roleid,0,0,Marshal::OctetsStream()<<Octets(fcname,strlen(fcname))) );
		Octets fc(fcname,strlen(fcname));
		Octets fc_ucs2;
		CharsetConverter::conv_charset_g2u(fc,fc_ucs2);
		bool retcode=false;
		manager->Send( sid,FactionOPRequest(_O_FACTION_CREATE,roleid,GenFOperParams(_O_FACTION_CREATE,create_param_ct(fc_ucs2).marshal(),retcode )) );
	}
	else if ( inputbuf[0]=='p' ) //appoint
	{
		int  dstroleid=0;
		char choice;
		int newrole=0;
		if ( 3!=sscanf(inputbuf,"%c %d %d",&choice,&dstroleid,&newrole) )
		{
			printf("\nAppoint params error. Usage: p <roleid> <new role>\n");
			goto BEGIN;
		}
		printf("Try to appoint %d to role %d....\n",dstroleid,newrole);
		manager->Send( sid,FactionOPRequest(_O_FACTION_APPOINT,roleid,appoint_param_ct(dstroleid,newrole).marshal()) );
	}
	else if ( inputbuf[0]=='m' ) //nickname
	{
		char choice;
		int  dstroleid=0;
		char fcname[64];
		if (3!=sscanf(inputbuf,"%c %d %32s",&choice,&dstroleid,fcname))
		{
			printf("\nRename params error. Usage: m <roleid> <new name>\n");
			goto BEGIN;
		}
		printf("Try to rename %d to %s\n",dstroleid,fcname);
		//manager->Send( sid,FactionOPRequest(_O_FACTION_CREATE,roleid,0,0,Marshal::OctetsStream()<<Octets(fcname,strlen(fcname))) );
		Octets fc(fcname,strlen(fcname));
		Octets fc_ucs2;
		CharsetConverter::conv_charset_g2u(fc,fc_ucs2);
		manager->Send( sid,FactionOPRequest(_O_FACTION_RENAME,roleid,rename_param_ct(dstroleid,fc_ucs2).marshal()) );

	}
	else if ( inputbuf[0]=='r' ) //resign 
	{
		char choice;
		if (1!=sscanf(inputbuf,"%c",&choice))
		{
			printf("\nResign params error. Usage: r\n");
			goto BEGIN;
		}
		printf("Try to resign....\n");
		manager->Send( sid,FactionOPRequest(_O_FACTION_RESIGN,roleid,resign_param_ct(0).marshal()) );		
	}
	else if ( inputbuf[0]=='s' ) //master resign
	{
		char choice;
		int  newmaster;
		if (2!=sscanf(inputbuf,"%c %d",&choice,&newmaster))
		{
			printf("\nMasterResign params error. Usage: s <newmaster>\n");
			goto BEGIN;
		}
		printf("Try master resign to %d\n",newmaster);
		manager->Send( sid,FactionOPRequest(_O_FACTION_MASTERRESIGN,roleid,masterresign_param_ct(newmaster).marshal()) );		
	}
	/*
	else if (inputbuf[0]=='j')
	{
		char choice;
		unsigned int factionid;
		if (2!=sscanf(inputbuf,"%c %d",&choice,&factionid))
		{
			printf("\njoin faction params error.\n");
			goto BEGIN;
		}
		printf("Try to join faction %d\n",factionid);
		manager->Send(sid, FactionOPRequest(_O_FACTION_APPLYJOIN,roleid,Marshal::OctetsStream()<<factionid ));
	}
	*/
	else if (inputbuf[0]=='d')
	{
		printf("Try to dismiss faction\n");
		//manager->Send(sid,FactionOPRequest(_O_FACTION_ACCEPTJOIN,roleid,Marshal::OctetsStream()<<applicant<<(char)blAgree) );	
		manager->Send( sid, FactionOPRequest(_O_FACTION_DISMISS,roleid,dismiss_param_ct().marshal()) );
	}

	else if (inputbuf[0]=='a')
	{
		char choice;
		int applicant;
		int factionid;
		int blAgree;
		if (3!=sscanf(inputbuf,"%c %d %d",&choice,&applicant,&factionid))
		{
			printf("\naccept params error. Usage: a <app_id> <fid>\n");
			goto BEGIN;
		}
		printf("Try to accept player %d, agree(%d)\n",applicant,blAgree);
		//manager->Send(sid,FactionOPRequest(_O_FACTION_ACCEPTJOIN,roleid,Marshal::OctetsStream()<<applicant<<(char)blAgree) );	
		manager->Send( sid, FactionAcceptJoin(roleid,sid,factionid,applicant) );
	}
	else if (inputbuf[0]=='e')
	{
		char choice;
		int member_id;
		if (2!=sscanf(inputbuf,"%c %d",&choice,&member_id))
		{
			printf("\naccept params error. Usage: e <member_id>\n");
			goto BEGIN;
		}
		printf("Try to expel member %d\n",member_id);
		manager->Send(sid,FactionOPRequest(_O_FACTION_EXPELMEMBER,roleid,Marshal::OctetsStream()<<member_id) );

	}
	else if (inputbuf[0]=='l')
	{
		int handle=0;
		printf("Try to list member\n");
		manager->Send(sid,FactionOPRequest(_O_FACTION_LISTMEMBER,roleid,Marshal::OctetsStream()<<handle) );
	}
	else if (inputbuf[0]=='v')
	{
		printf("Try to leave faction\n");
		manager->Send(sid,FactionOPRequest(_O_FACTION_LEAVE,roleid,leave_param_ct().marshal() ) );
	}

	else if (inputbuf[0]=='c')
	{
		char choice;
		char msg[256];
		if (2!=sscanf(inputbuf,"%c %128s",&choice,msg))
		{
			printf("\nchat params error. Usage: c <msg>\n");
			goto BEGIN;
		}
		printf("Try to send factionchat msg: %s\n",msg);
		manager->Send(sid,FactionChat(GN_CHAT_CHANNEL_FACTION, 0, roleid,Octets(msg,strlen(msg))));
		goto BEGIN;
	}
	else
	{
		printf("\nInput error. Try again.\n");
		goto BEGIN;
	}
}

};
#endif
