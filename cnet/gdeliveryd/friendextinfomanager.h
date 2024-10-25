#ifndef __GNET_FRIENDEXTINFOMANAGER_H
#define __GNET_FRIENDEXTINFOMANAGER_H

#include <vector>
#include <map>
#include "mapuser.h"
#include "gfriendextinfo"

#define AUMAIL_LEVEL_LIMIT 70
namespace GNET
{
	class FriendextinfoManager
	{
		typedef struct
		{
			int login_time;
			int level;
			int uid;
			int reincarnation_times;
		}ExtInfo;
		typedef std::map<int,ExtInfo> RoleLoginTimeMap;
		
		private:
			bool permission;
			RoleLoginTimeMap rolelogintimelist;
		
			//用于更新pinfo中的FriendExt信息
			void UpdateGFriendExt(GFriendExtInfoVector::iterator iter,int uid,int login_time,int level,int now_time, int reincarnation_times);
			//发送好友额外信息到客户端
			void SendFriendExt2Client(const PlayerInfo *pinfo);
			//根据离线天数计算奖励等级
			int  GetBonusLevel(const int day);
			//回馈发送邮件结果到客户端
			void SendMailResult2Client(const PlayerInfo * pinfo,const int result,const int friend_id);	
			//在cache里面查询role的登录时间
			void FindRoleLoginTime(std::vector<int> &roleidlist,PlayerInfo *pinfo);
			//只从cache里面更新pinfo，用于发信时
			void UpdateLoginTimeFromCache(PlayerInfo *pinfo);
		
		public:
			FriendextinfoManager():permission(false)
			{
			}
			~FriendextinfoManager() { }

			static FriendextinfoManager* GetInstance()
			{
				static FriendextinfoManager _instance;
				return &_instance;
			}

			void Initialize(bool recall);
			//玩家上线的时候更新cache
			void UpdateRoleLoginTime(PlayerInfo *pinfo,const int logintime);
			//DB返回数据之后更新cache 和 pinfo
			void UpdateRoleLoginTime(GFriendExtInfoVector friend_list,PlayerInfo *pinfo); 
			void SearchAllExt(PlayerInfo *pinfo);
			void SendAUMail(PlayerInfo *pinfo,int friend_id,int mail_template_id);
	};
};

#endif
