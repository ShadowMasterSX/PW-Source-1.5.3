#include <time.h>

#include "timer.h"
#include "gproviderserver.hpp"
#include "conv_charset.h"

#include "friendextinfomanager.h"
#include "mapuser.h"
#include "rpcdefs.h"
#include "dbfriendextlist.hpp"
#include "friendextlist.hpp"
#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "sendaumail_re.hpp"
#include "game2au.hpp"
#include "aumailsended.hpp"
#include "gauthclient.hpp"
#include <algorithm>
//在cache里面查询role的登录时间
namespace GNET
{
		
	void FriendextinfoManager::SearchAllExt(PlayerInfo *pinfo)
	{
		int now = Timer::GetTime();
		std::vector<int> roleidlist;//需要更新上线时间的好友id表，会去cache或者数据库中取
		if(!pinfo->friends.size())
			return;
		
		GFriendInfoVector::iterator gfriend_iter = pinfo->friends.begin();
		for(;gfriend_iter!=pinfo->friends.end();gfriend_iter++)
		{
			GFriendExtInfoVector::iterator gfriendext_iter = pinfo->friendextinfo.begin();
			for(;gfriendext_iter != pinfo->friendextinfo.end();gfriendext_iter++)
			{
				if(gfriend_iter->rid == gfriendext_iter->rid)
				{
					break;
				}
			}
			//玩家好友额外信息列表中没有该好友数据
			if(gfriendext_iter == pinfo->friendextinfo.end())  
			{
				roleidlist.push_back(gfriend_iter->rid);
			}
			//有该好友数据但是过旧
			else if(now - gfriendext_iter->update_time > _SECONDS_ONE_DAY)
			{
				roleidlist.push_back(gfriend_iter->rid);
			}
		}
		
		if(!roleidlist.size()) //如果没有需要向cache或者数据库中查找的玩家数据
		{
			SendFriendExt2Client(pinfo);
		}
		else
		{
			//去cache中取
			FindRoleLoginTime(roleidlist,pinfo);
		}
	}
	
	void FriendextinfoManager::FindRoleLoginTime(std::vector<int> &roleidlist,PlayerInfo *pinfo)
	{
		//roleidlist是需要更新的好友上线时间的id列表
		if(!roleidlist.size()) return;
		std::vector<int> miss_vec;
		std::map<int,ExtInfo> logintimelist;
		//在manager的缓存中查找上线时间
		for(size_t i=0;i<roleidlist.size();i++)
		{
			RoleLoginTimeMap::iterator irolelogintime;
			irolelogintime = rolelogintimelist.find(roleidlist[i]);
			if(irolelogintime == rolelogintimelist.end())
			{
				//将找不到的放入miss表
				miss_vec.push_back(roleidlist[i]);
			}
			else
			{
				//将找到的放入查找表
				logintimelist[roleidlist[i]] = irolelogintime->second;
			}
		}
		//将查找到的好友更新到pinfo中
		if(logintimelist.size())
		{
			std::map<int,ExtInfo>::iterator logintime_iter = logintimelist.begin();
			int now = Timer::GetTime();
			//遍历查找表
			for(;logintime_iter!=logintimelist.end();logintime_iter++)
			{
				GFriendExtInfoVector::iterator gfriendext_iter = pinfo->friendextinfo.begin();
				for(;gfriendext_iter!=pinfo->friendextinfo.end();gfriendext_iter++)
				{
					if(logintime_iter->first == gfriendext_iter->rid)
					{
						//找到该好友则更新
						UpdateGFriendExt(gfriendext_iter,logintime_iter->second.uid,logintime_iter->second.login_time,logintime_iter->second.level,now,logintime_iter->second.reincarnation_times);
						break;
					}
				}
				//没找到则添加
				if(gfriendext_iter == pinfo->friendextinfo.end())
				{
					GFriendExtInfo tmp;
					tmp.rid = logintime_iter->first;
					tmp.last_logintime = logintime_iter->second.login_time;
					tmp.update_time = now;
					tmp.level = logintime_iter->second.level;
					tmp.uid = logintime_iter->second.uid;
					tmp.reincarnation_times = logintime_iter->second.reincarnation_times;
					pinfo->friendextinfo.push_back(tmp);
				}
			}
			pinfo->friend_ver ++;
		}

		if(!miss_vec.size())
		{
			//发送给客户端
			SendFriendExt2Client(pinfo);
		}
		else
		{
			//如果存在没有找到的则去数据库里查找
			DBFriendExtList dbpro;
			dbpro.rid = pinfo->roleid;
			dbpro.roleidlist.swap(miss_vec);
			GameDBClient::GetInstance()->SendProtocol(dbpro);
		}
		return ;
	}


	void FriendextinfoManager::UpdateRoleLoginTime(PlayerInfo * pinfo,const int logintime)
	{
		rolelogintimelist[pinfo->roleid].login_time = logintime;
		rolelogintimelist[pinfo->roleid].level = pinfo->level;
		rolelogintimelist[pinfo->roleid].uid = pinfo->userid;
		rolelogintimelist[pinfo->roleid].reincarnation_times = pinfo->reincarnation_times;
	}

	void FriendextinfoManager::UpdateRoleLoginTime(GFriendExtInfoVector friend_list,PlayerInfo *pinfo)
	{
		int now = Timer::GetTime();
		GFriendExtInfoVector::iterator gfeiv_iter = friend_list.begin();
		//更新cache
		for(;gfeiv_iter != friend_list.end(); gfeiv_iter++)
		{
			rolelogintimelist[gfeiv_iter->rid].login_time = gfeiv_iter->last_logintime;
			rolelogintimelist[gfeiv_iter->rid].level = gfeiv_iter->level;
			rolelogintimelist[gfeiv_iter->rid].uid = gfeiv_iter->uid;
			rolelogintimelist[gfeiv_iter->rid].reincarnation_times = gfeiv_iter->reincarnation_times;
		}
		gfeiv_iter = friend_list.begin();
		//更新pinfo
		for(;gfeiv_iter != friend_list.end(); gfeiv_iter++)
		{
			GFriendExtInfoVector::iterator gfriendext_iter = pinfo->friendextinfo.begin();
			for(;gfriendext_iter != pinfo->friendextinfo.end(); gfriendext_iter++)
			{
				if(gfriendext_iter->rid == gfeiv_iter->rid)
				{
					UpdateGFriendExt(gfriendext_iter,gfeiv_iter->uid,gfeiv_iter->last_logintime,gfeiv_iter->level,now,gfeiv_iter->reincarnation_times);
					break;
				}
			}
			if(gfriendext_iter == pinfo->friendextinfo.end())
			{
				GFriendExtInfo info = *gfeiv_iter;
				pinfo->friendextinfo.push_back(info);
			}
		}
		pinfo->friend_ver++;
		SendFriendExt2Client(pinfo);
	}
	
	class EventIsInSendAUMailInfo
	{
	public:
		EventIsInSendAUMailInfo(const int rid):s_rid(rid){}
		bool operator() (const std::vector<GFriendInfo>::value_type &gfriend)
		{
			return gfriend.rid == s_rid;
		}
	private:
		int s_rid;
	};

	void FriendextinfoManager::SendAUMail(PlayerInfo *pinfo,int friend_id,int mail_template_id)
	{
		if(!permission)
		{
			SendMailResult2Client(pinfo,ERR_AUMAIL_NOTENABLE,friend_id);
			return;
		}
		time_t now = Timer::GetTime();
		struct tm* dt = localtime(&now);
		dt->tm_sec = 0;
		dt->tm_min = 0;
		dt->tm_hour = 0;
		int day_time = mktime(dt);
		GFriendExtInfoVector::iterator gfeiv_iter = pinfo->friendextinfo.begin();
		GSendAUMailRecordVector::iterator send_iter = pinfo->sendaumailinfo.begin();
		int count = 0;
		//通过cache更新一次玩家的friendextinfo，防止数据过老出现bug
		UpdateLoginTimeFromCache(pinfo);
		if(pinfo->level < 90)
		{
			return;
		}
		for(;gfeiv_iter != pinfo->friendextinfo.end();gfeiv_iter++)
		{
			//判断该好友是否存在
			if(gfeiv_iter->rid == friend_id)
			{
				break;
			}
		}
		//没有该好友
		if(gfeiv_iter == pinfo->friendextinfo.end())
		{
			SendMailResult2Client(pinfo,ERR_AUMAIL_NOTFOUND,friend_id);
			return;
		}
		//好友等级过低
		if(gfeiv_iter->level < AUMAIL_LEVEL_LIMIT)
		{
			SendMailResult2Client(pinfo,ERR_AUMAIL_LEVELLOW,friend_id);	
			return;
		}
		//离开时间限制
		if(((now-gfeiv_iter->last_logintime)/_SECONDS_ONE_DAY) < 16)
		{
			SendMailResult2Client(pinfo,ERR_AUMAIL_NOTSOLONG,friend_id);
			return;
		}
		
		//判断CD
		for(;send_iter != pinfo->sendaumailinfo.end();send_iter++)
		{
			if(send_iter->rid == friend_id)
			{
				//10天CD
				if(now - send_iter->sendmail_time < _SECONDS_ONE_DAY *10)
				{
					SendMailResult2Client(pinfo,ERR_AUMAIL_TENCD,friend_id);
					return;
				}
			}
			//统计一天内发送过的数量,每天0点为更新时间
			if(day_time < send_iter->sendmail_time)
			{
				count ++;
			}
		}
		//5人CD
		if(count >= 5)
		{
			SendMailResult2Client(pinfo,ERR_AUMAIL_FIVECD,friend_id);
			return;
		}

		//清理sendaumailinfo
		if(pinfo->sendaumailinfo.size() > 50)
		{
			GSendAUMailRecordVector::iterator sendaumail_iter = pinfo->sendaumailinfo.begin();
			while(sendaumail_iter != pinfo->sendaumailinfo.end())
			{
				//删除记录中天数大于10的
				if(now - sendaumail_iter->sendmail_time > _SECONDS_ONE_DAY*10)
				{
					sendaumail_iter = pinfo->sendaumailinfo.erase(sendaumail_iter);
				}
				else
				{
					//删除记录中天数大于1且不是好友的
					GFriendInfoVector::iterator temp_it = std::find_if(pinfo->friends.begin(),pinfo->friends.end(),EventIsInSendAUMailInfo(sendaumail_iter->rid));
					if(temp_it == pinfo->friends.end())
					{
						if(day_time > sendaumail_iter->sendmail_time)
						{
							sendaumail_iter=pinfo->sendaumailinfo.erase(sendaumail_iter);
						}
						else
						{
							sendaumail_iter++;
						}
					}
					else
					{
						sendaumail_iter++;
					}
				}
			}
		}
		
		//通知客户端
		SendMailResult2Client(pinfo,ERR_SUCCESS,friend_id);

		//添加到已发送邮件列表
		GSendAUMailRecordVector::iterator gsaurv_iter = pinfo->sendaumailinfo.begin();
		for(;gsaurv_iter != pinfo->sendaumailinfo.end();gsaurv_iter++)
		{
			if(gsaurv_iter->rid == gfeiv_iter->rid)
			{
				gsaurv_iter->sendmail_time = now;
				break;
			}
		}
		if(gsaurv_iter == pinfo->sendaumailinfo.end())
		{
			GSendAUMailRecord gsaur;
			gsaur.rid = gfeiv_iter->rid;
			gsaur.sendmail_time = now;
			pinfo->sendaumailinfo.push_back(gsaur);
		}
		pinfo->friend_ver++;
		
		//取得该好友信息
		GFriendInfoVector::iterator gfriend_iter = 	pinfo->friends.begin();
		for(;gfriend_iter!=pinfo->friends.end();gfriend_iter++)
		{
			if(gfriend_iter->rid == gfeiv_iter->rid)
			{
				break;
			}
		}
	
		//发送到AU
		Game2AU g2a;
		g2a.userid = pinfo->userid;
		g2a.qtype = 1;
		Marshal::OctetsStream data;
		data << gfeiv_iter->uid << pinfo->name << gfriend_iter->name << mail_template_id;
		g2a.info = data;
		GAuthClient::GetInstance()->SendProtocol( g2a );
		
		int send_lvl = GetBonusLevel((now - gfeiv_iter->last_logintime)/_SECONDS_ONE_DAY);

		char ex_reward = (count == 4)?1:0;
		
		Log::formatlog("SendAUMail","send_userid=%d:send_role=%d:received_userid=%d:received_roleid=%d:level=%d:offline_seconds=%d:bonus_lvl=%d:ex_reward=%d",g2a.userid,pinfo->roleid,gfeiv_iter->uid,gfeiv_iter->rid,gfeiv_iter->level,gfeiv_iter->last_logintime,send_lvl,ex_reward);	

		//发送协议到gamed以触发奖励


		//发送到gamed
		AUMailSended ams;
		ams.roleid = pinfo->roleid;
		ams.level = send_lvl;
		ams.ext_reward = ex_reward;
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->user->gameid,ams);

		return;
	}
	void FriendextinfoManager::SendMailResult2Client(const PlayerInfo *pinfo,const int result,const int friend_id)
	{
		SendAUMail_Re re;
		re.roleid = friend_id;
		re.result = result;
		re.localsid =pinfo->localsid;
		GDeliveryServer::GetInstance()->Send( pinfo->linksid,re); 
	}
	
	void FriendextinfoManager::SendFriendExt2Client(const PlayerInfo *pinfo)
	{
		FriendExtList pro;
		pro.extra_info = pinfo->friendextinfo;
		pro.roleid = pinfo->roleid;
		pro.send_info = pinfo->sendaumailinfo;
		pro.localsid = pinfo->localsid;
		GDeliveryServer::GetInstance()->Send( pinfo->linksid,pro);
	}

	int FriendextinfoManager::GetBonusLevel(const int day)
	{
		if(day <=20 && day > 15)
		{
			return 1;
		}
		if(day <=30 && day > 20)
		{
			return 2;
		}
		if(day > 30)
		{
			return 3;	
		}
		return 0;
	}
	
	void FriendextinfoManager::UpdateGFriendExt(GFriendExtInfoVector::iterator iter,int uid,int login_time,int level,int now_time, int reincarnation_times)
	{
		iter->uid = uid;
		iter->level = level;
		iter->last_logintime = login_time;
		iter->update_time = now_time;
		iter->reincarnation_times = reincarnation_times;
		return;
	}
	
	void FriendextinfoManager::UpdateLoginTimeFromCache(PlayerInfo *pinfo)
	{
		bool sendExtList2Client = false;
		GFriendExtInfoVector::iterator gfeiv_iter = pinfo->friendextinfo.begin();
		int now = Timer::GetTime();
		for(;gfeiv_iter != pinfo->friendextinfo.end();gfeiv_iter++)
		{
			RoleLoginTimeMap::iterator irolelogintime;
			irolelogintime = rolelogintimelist.find(gfeiv_iter->rid);
			if(irolelogintime != rolelogintimelist.end())
			{
				if(gfeiv_iter->last_logintime != irolelogintime->second.login_time)
				{
					gfeiv_iter->last_logintime = irolelogintime->second.login_time;
					gfeiv_iter->update_time = now;
					sendExtList2Client = true;
				}
			}
		}
		if(sendExtList2Client)
		{
			SendFriendExt2Client(pinfo);
		}
	}

	void FriendextinfoManager::Initialize(bool recall)
	{
		permission = recall;
		return;
	}

}
