#ifndef __GNET_GMCHOICE_H
#define __GNET_GMCHOICE_H
#include <stdio.h>
#include "macros.h"

#include "glinkclient.hpp"
#include "conv_charset.h"

#include "gmonlinenum.hpp"
#include "gmlistonlineuser.hpp"
#include "gmkickoutuser.hpp"
#include "gmshutup.hpp"
#include "gmkickoutrole.hpp"
#include "gmshutuprole.hpp"

#include "gmrestartserver.hpp"
#include "privatechat.hpp"
#include "publicchat.hpp"
#include "getplayeridbyname.hpp"
#include "gmtogglechat.hpp"
#include "gmforbidrole.hpp"
#include "localmacro.h"
namespace
{
	void GMChoice(int roleid,Protocol::Manager* manager,Protocol::Manager::Session::ID sid)
	{
BEGIN:
		char inputbuf[256];
		printf("GameMaster operation: online_num/list user/chat/talk/announce/kick/query role/shutup/restart/togglechat/forbid?(n/l/c/t/a/k/q/s/r/g/f)");
		fflush(stdout);
		fgets(inputbuf,100,stdin);
		inputbuf[strlen(inputbuf)-1]='\0';
		if (inputbuf[0]=='n')
		{
			manager->Send(sid,GMOnlineNum(roleid,sid));	
		}
		else if (inputbuf[0]=='l')
		{
			char choice;
			int handler;
			if ( 2!=sscanf(inputbuf,"%c %d",&choice,&handler) )
			{
				printf("\nlist user params error.\n");
				goto BEGIN;
			}
			printf("Query onlineuser to server.\n");
			manager->Send(sid,GMListOnlineUser(roleid,sid,handler,Octets()));
		}
		else if (inputbuf[0]=='c')
		{
			char choice;
			Octets srcname_gbk,srcname_ucs2;
			char dstname[256];
			Octets dstname_gbk,dstname_ucs2;
			char msg[256];
			Octets msg_gbk,msg_ucs2;
			
			if ( 3!=sscanf(inputbuf,"%c %s %s",&choice,dstname,msg) )
			{
				printf("\nprivate chat params error.\n");
				goto BEGIN;
			}
			srcname_gbk=Octets("GameMaster",10);
			//conv_charset("GBK","UCS2",srcname_gbk,srcname_ucs2);
			CharsetConverter::conv_charset_g2u(srcname_gbk,srcname_ucs2);

			dstname_gbk=Octets(dstname,strlen(dstname));
			//conv_charset("GBK","UCS2",dstname_gbk,dstname_ucs2);
			CharsetConverter::conv_charset_g2u(dstname_gbk,dstname_ucs2);
			
			msg_gbk=Octets(msg,strlen(msg));
			//conv_charset("GBK","UCS2",msg_gbk,msg_ucs2);
			CharsetConverter::conv_charset_g2u(msg_gbk,msg_ucs2);
			printf("try to send msg(%s) to player %s\n",msg,dstname);
			manager->Send(sid,PrivateChat(GN_CHAT_CHANNEL_WHISPER,0,srcname_ucs2,roleid,dstname_ucs2,0,msg_ucs2,Octets()));
			goto BEGIN;
		}
		else if (inputbuf[0]=='t')
		{
			char choice;
			Octets srcname_gbk,srcname_ucs2;
			int dstroleid=0;
			Octets dstname_gbk,dstname_ucs2;
			char msg[256];
			Octets msg_gbk,msg_ucs2;
			
			if ( 3!=sscanf(inputbuf,"%c %d %s",&choice,&dstroleid,msg) )
			{
				printf("\nprivate talk params error.\n");
				goto BEGIN;
			}
			srcname_gbk=Octets("GameMaster",10);
			//conv_charset("GBK","UCS2",srcname_gbk,srcname_ucs2);
			CharsetConverter::conv_charset_g2u(srcname_gbk,srcname_ucs2);
			{
				GLinkClient* lcm=GLinkClient::GetInstance();
				Thread::Mutex::Scoped l(lcm->locker_rolenamemap);
				GLinkClient::RolenameMap::iterator it=lcm->rolenamemap.find(dstroleid);
				if (it==lcm->rolenamemap.end())
				{
					printf("\ncan not find name of role %d.\n",dstroleid);
					goto BEGIN;
				}
				dstname_ucs2=(*it).second;
				//conv_charset("UCS2","GBK",dstname_ucs2,dstname_gbk);
				CharsetConverter::conv_charset_u2g(dstname_ucs2,dstname_gbk);
			}	
			msg_gbk=Octets(msg,strlen(msg));
			//conv_charset("GBK","UCS2",msg_gbk,msg_ucs2);
			CharsetConverter::conv_charset_g2u(msg_gbk,msg_ucs2);
			printf("try to send msg(%s) to player %.*s(%d)\n",msg,dstname_gbk.size(),(char*)dstname_gbk.begin(),dstroleid);
			manager->Send(sid,PrivateChat(GN_CHAT_CHANNEL_WHISPER,0,srcname_ucs2,roleid,dstname_ucs2,dstroleid,msg_ucs2,Octets()));		
			goto BEGIN;
		}

		else if (inputbuf[0]=='a')
		{
			char choice;
			char msg[256];
			Octets msg_gbk,msg_ucs2;
			
			if ( 2!=sscanf(inputbuf,"%c %s",&choice,msg) )
			{
				printf("\nBroadcast chat params error.\n");
				goto BEGIN;
			}
			msg_gbk=Octets(msg,strlen(msg));
			//conv_charset("GBK","UCS2",msg_gbk,msg_ucs2);
			CharsetConverter::conv_charset_g2u(msg_gbk,msg_ucs2);
		
			printf("try to broadcast msg(%s)\n",msg);
			manager->Send(sid,PublicChat(GN_CHAT_CHANNEL_BROADCAST,0,roleid,sid,msg_ucs2,Octets()));
			goto BEGIN;
		}
		else if (inputbuf[0]=='k')
		{
			char choice;
			int kickeduserid;
			int time;
			if ( 3!=sscanf(inputbuf,"%c %d %d",&choice,&kickeduserid,&time) )
			{
				printf("\nKickuser params error. \n");
				goto BEGIN;
			}
			printf("try to kickout role %d, forbid time is %d(seconds)\n",kickeduserid,time*60);
			Octets msg_gbk("½ûÖ¹µÇÂ½",8);
			//Octets msg_gbk("Forbid Login",12);
			//manager->Send(sid,GMKickoutRole(roleid,sid,kickeduserid,time*60,msg_gbk));
			manager->Send(sid,GMKickoutUser(roleid,sid,kickeduserid,time*60,msg_gbk));
		}
		else if(inputbuf[0]=='q')
		{
			char choice;
			char name[256];
			Octets name_gbk,name_ucs2;
			if (2!=sscanf(inputbuf,"%c %s",&choice,name))
			{
				printf("\nQuery params error. \n");
				goto BEGIN;
			}
			name_gbk=Octets(name,strlen(name));
			//conv_charset("GBK","UCS2",name_gbk,name_ucs2);
			CharsetConverter::conv_charset_g2u(name_gbk,name_ucs2);
			printf("try to query role %s\n",name);
			manager->Send(sid,GetPlayerIDByName(name_ucs2,sid));
		}
		else if (inputbuf[0]=='s')
		{
			char choice;
			int userid;
			int time;
			if ( 3!=sscanf(inputbuf,"%c %d %d",&choice,&userid,&time) )
			{
				printf("\nShutup params error. \n");
				goto BEGIN;
			}
			printf("try to Shutup role %d, forbid time is %d(seconds)\n",userid,time*60);
			Octets msg_gbk("½ûÖ¹·¢ÑÔ",8);
			//Octets msg_gbk("Forbid Talk",12);
			//manager->Send(sid,GMShutupRole(roleid,sid,userid,time*60,msg_gbk));
			manager->Send(sid,GMShutup(roleid,sid,userid,time*60,msg_gbk));
		}
		else if (inputbuf[0]=='f')
		{
			char choice;
			int userid;
			int time;
			int type;
			if ( 4!=sscanf(inputbuf,"%c %d %d %d",&choice,&userid,&type,&time) )
			{
				printf("\nForbid params error. \n");
				goto BEGIN;
			}
			printf("try to Forbid role %d, forbid time is %d(seconds),forbidtype is %d,\n",userid,time*60,type);
			Octets msg_gbk("Ïë·â¾Í·â",8);
			manager->Send(sid,GMForbidRole((char)type,roleid,sid,userid,time*60,msg_gbk));
		}

		else if (inputbuf[0]=='r')
		{
			char choice;
			int time;
			if ( 2!=sscanf(inputbuf,"%c %d",&choice,&time) )
			{
				printf("\nRestart server params error. \n");
				goto BEGIN;
			}
			printf("try to restart server in %d seconds\n",time);
			manager->Send(sid,GMRestartServer(roleid,sid,0,time));
			goto BEGIN;
		}
		else if (inputbuf[0]=='g')
		{
			char choice;
			int enable;
			if ( 2!=sscanf(inputbuf,"%c %d",&choice,&enable) )
			{
				printf("\nToggle chat params error. \n");
				goto BEGIN;
			}
			printf("try to toggle chat to enable=%d\n",enable);
			manager->Send(sid,GMToggleChat(roleid,sid,(char)enable));
			goto BEGIN;

		}
		else
		{
			printf("\nInput error.\n");
			goto BEGIN;
		}
			
	}
};
#endif
