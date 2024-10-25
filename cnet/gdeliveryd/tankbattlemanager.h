#ifndef __GNET_TANK_BATTLE_MANAGER_H__
#define __GNET_TANK_BATTLE_MANAGER_H__

#include <vector>
#include <map>
#include <functional>

#include "itimer.h"
#include "tankbattleplayerscoreinfo"

namespace GNET
{

namespace TankBattleConfig
{
const char split_tag = ';';		//拆分config里字符串的标记
const bool only_one_battle_server = false;	//是否只允许开启一个战车战场服务器
const size_t assign_count_each_tick = 10;	//每次tick给多少玩家分配战场,0的话就是没有限制
const int player_switch_time_out = 120;		//玩家跳转地图的超时时间
const int server_create_time_out = 30;		//通知gs开启新战场的超时时间
const int server_error_time_out = 60;		//发生错误的重置时间
const int battle_man_wait_bonus_time = 600;	//等待所有房间关闭的时间
const int battle_man_wait_bonus_time_out = 120;	//等待所有房间关闭的超时时间
const int battle_man_before_close_time = 3600;	//进入关闭前的结算和等待的总时间
const int battle_close_clear_time = 60;	//战场结束多长时间之后清空本战场的玩家信息
const int battle_close_time_out = 60;	//战场开启之后结束的超时时间
const int max_battle_count = 50;	//每个副本服务器最多能开辟战场的数量
const int rank_show_count = 10;		//客户端显示排行榜的前多少名
}

class TankBattleManager : public IntervalTimer::Observer
{
private:

	enum SERVER_STAT
	{
		SERVER_STAT_NORMAL,		//正常状态，可以开辟新的战场副本
		SERVER_STAT_CREATE,		//正在开辟新的副本过程中
		SERVER_STAT_DISCONNECT,	//服务器已经断线，先不分配新的战场
		SERVER_STAT_ERROR,		//服务器发生错误
		SERVER_STAT_FULL,		//服务器已经副本数量上限
	};
	
	enum BATTLE_MAN_STAT
	{
		BATTLE_MAN_STAT_CLOSE,		//活动没有开启
		BATTLE_MAN_STAT_OPEN,		//活动处于开启状态
		BATTLE_MAN_STAT_WAIT_BONUS,	//活动已经结束，等待所有战场房间结束之后结算
		BATTLE_MAN_STAT_BONUS,		//所有房间都已经结束，开始结算和发奖
		BATTLE_MAN_STAT_WAIT_CLOSE,	//发奖已经结束，等待进入关闭阶段
	};

	enum BATTLE_STAT
	{
		BATTLE_STAT_CREATE,		//正在创建的过程中
		BATTLE_STAT_OPEN,		//处于开启状态
		BATTLE_STAT_WAIT_CLOSE,	//战场副本刚结束
		BATTLE_STAT_CLOSE,		//处于关闭状态
	};

	enum PLAYER_STAT
	{
		PLAYER_STAT_APPLY,		//玩家申请加入战场，等待分配
		PLAYER_STAT_SWITCH,		//玩家正在跳转进副本
		PLAYER_STAT_ENTER,		//玩家已经进入了战场
		PLAYER_STAT_LEAVE,		//玩家离开了战场
	};
	
	struct ServerInfo
	{
		int world_tag;
		int server_id;
		int battle_count;   //当前开了多少个战场
		int time_stamp;		//server操作的一个时间戳
		SERVER_STAT stat;	//当前的开启状态

		ServerInfo()
		{
			world_tag = 0;
			server_id = 0;
			battle_count = 0;
			time_stamp = 0;
			stat = SERVER_STAT_NORMAL;
		}
	};
	
	struct BattleInfo
	{
		int battle_id;
		int world_tag;
		int end_time;		//战场结束的时间点
		int time_stamp;		//状态变化的时间戳
		BATTLE_STAT status;

		std::vector<int> players;

		BattleInfo()
		{
			battle_id = 0;
			world_tag = 0;
			end_time = 0;
			time_stamp = 0;
			status = BATTLE_STAT_CLOSE;
		}

		inline bool DelPlayer(int roleid)
		{
			std::vector<int>::iterator it = players.begin(),eit=players.end();
			for( ; it != eit; ++it)
			{
				if (*it == roleid)
				{
					players.erase(it);
					return true;
				}
			}
			return false;
		}
	};
	
	struct PlayerApplyEntry
	{
		int gameid;		//玩家报名的时候所在的gameid，用来和分配的时候进行验证，如果换过地图了不让进
		int model;		//玩家报名的时候选择的模型
	};
	
	struct PlayerEntry
	{
		int roleid;
		int battle_id;
		time_t time_stamp;
		PLAYER_STAT stat;

		PlayerEntry()
		{
			roleid = 0;
			battle_id = 0;
			time_stamp = 0;
			stat = PLAYER_STAT_APPLY;
		}
	};
	
	struct PlayerBonus
	{
		int roleid;
		int rank;
		int bonus;
	};

	struct BonusEntry
	{
		int rank;
		int count;
	};
	
	typedef std::map<int/*roleid*/, PlayerApplyEntry> PLAYER_APPLY_MAP;
	typedef std::map<int/*world_tag*/, ServerInfo> SERVER_INFO_MAP;
	typedef std::map<int/*battleid*/, BattleInfo> BATTLE_MAP;
	typedef std::map<int/*roleid*/, PlayerEntry> PLAYER_ENTRY_MAP;
	typedef std::map<int/*roleid*/, TankBattlePlayerScoreInfo> PLAYER_SCORE_MAP;
	typedef std::vector<PlayerBonus> PLAYER_BONUS_VEC;
	typedef std::multimap<int/*score*/, TankBattlePlayerScoreInfo * , std::greater<int> > PLAYER_SCORE_RANK_MAP;
	typedef std::vector<const TankBattlePlayerScoreInfo *> TOP_SCORE_VEC;
	typedef std::vector<BonusEntry> BONUS_ENTRY_VEC;

	TankBattleManager():_status(BATTLE_MAN_STAT_CLOSE),_battle_index(1),_start_time(0),_end_time(0),
			_adjust_time(0),_min_time(0),_max_time(0),_max_player(0),_no_new_battle_time(0),_cant_enter_time(0),
			_second_of_day(0),_send_bonus_per_second(0),_calc_bonus(false),_bonus_item_id(0),_bonus_max_count(0),_bonus_proctype(0)
	{
		memset(_open_days,0,sizeof(_open_days));
	}

public:

	static TankBattleManager * GetInstance()
	{
		static TankBattleManager instance;
		return &instance;
	}

	bool Initialize();

	//重置战场信息
	void ResetBattleMgr();
	
	//设置当前修正时间
	inline void SetAdjustTime(int t) {_adjust_time = t;}

	//设置战场最大人数,调用前需要验证传入参数的有效性
	inline void SetMaxPlayer(int n) {_max_player = n;}

	//设置战场最短时间和最长时间,只对设置以后新开启的战场有效,调用前需要验证传入参数的有效性
	inline void SetMinAndMaxTime(int min, int max) {_min_time=min;_max_time=max;}
	
	//战场副本gs发送注册消息
	void RegisterServerInfo(int world_tag, int server_id);

	//战场开启，发信息给gs申请副本
	void StartBattle();

	//战场开启之后gs发送返回的回调
	void OnBattleStart(int world_tag, int battleid, int retcode);

	//gs通知战场结束
	void OnBattleEnd(int world_tag, int battleid);

	//gs断开连接，在函数里检测是否战场
	void DisableServerInfo(int world_tag);

	//玩家报名参加战场
	void PlayerApply(int roleid, int model);

	//玩家成功进入战场
	void PlayerEnterBattle(int roleid, int battle_id, int world_tag);

	//玩家离开战场，包含下线
	void PlayerLeaveBattle(int roleid, int battle_id, int world_tag);
	
	//更新玩家积分信息
	void PlayerScoreUpdate(int battle_id, int world_tag, const TankBattlePlayerScoreInfoVector & player_scores);

	//玩家查询排行榜
	int PlayerGetRank(int roleid, TankBattlePlayerScoreInfo & self_score, 
			TankBattlePlayerScoreInfoVector & player_scores) const;

private:

	bool Update();

	//给客户端发送玩家申请加入战场的结果，在函数里边根据roleid查找玩家在不在线
	void SendPlayerApplyRe(int roleid, int retcode, unsigned int linksid = 0, unsigned int localsid = 0);

	//更新战场服务器状态信息
	void UpdateServerInfo(time_t nowtime);

	//更新战场副本信息
	void UpdateBattleInfo(time_t nowtime);

	//更新玩家信息
	void UpdatePlayerInfo(time_t nowtime);

	//计算玩家排名(所有玩家全部重新计算，尽量少调用)
	void CalcPlayerRank();

	//计算所有玩家奖励
	void CalcAllBonus();

	//计算单独玩家奖励
	int CalcPlayerBonus(int rank);

	//遍历奖励列表给玩家发奖
	int SendAllBonus();

	//给单独玩家发送奖励
	void SendPlayerBonus(int roleid, int player_bonus, int rank);

	//选取一个人数最多的有空余位置的战场
	BattleInfo * GetMostPlayerAndHaveVacancyBattle();

	//根据battle_id获取battle_info
	BattleInfo * GetBattleByBattleID(int battle_id);

	//根据world_tag获取server_info
	ServerInfo * GetServerInfoByWorldTag(int world_tag);

	//根据roleid获取player_entry
	PlayerEntry * GetPlayerEntryByRoleID(int roleid);

	//获取当前时间
	int GetTime() const;

	//获取空闲battle_id
	inline int GetFreeBattleID() { return _battle_index++;}

private:

	BATTLE_MAN_STAT _status;	//当前状态
	int _battle_index;	//当前战场索引,递增值
	int _start_time;	//活动当天开始时间，从0点计时的秒数
	int _end_time;		//活动当天结束时间，从0点计时的秒数
	int _adjust_time;	//时间的修正值，用来调试(秒)
	int _min_time;		//一场战斗的最短时间(秒)
	int _max_time;		//一场战斗的最大时间(秒)
	int _max_player;	//战场的最大人数
	int _no_new_battle_time;	//距离活动结束多长时间不开放新战场(秒)
	int _cant_enter_time;	//距离每个战场副本结束多长时间不允许进入该战场(秒)
	int _second_of_day;		//当前时间是从0点开始的多少秒,每次Update更新
	int _send_bonus_per_second;	//每秒发放奖励的数量
	bool _calc_bonus;		//是否已经计算过奖励
	unsigned char _open_days[7];	//活动每周周几开启(0 当天不开启 1当天开启)
	int _bonus_item_id;		//活动奖励物品的id
	int _bonus_max_count;	//奖励物品的叠加数量上限
	int _bonus_proctype;	//物品的绑定类型

	BATTLE_MAP _battle_map;
	SERVER_INFO_MAP _server_info_map;
	PLAYER_APPLY_MAP _player_apply_map;
	PLAYER_ENTRY_MAP _player_entry_map;
	PLAYER_SCORE_MAP _player_score_map;
	PLAYER_BONUS_VEC _player_bonus_vec;
	BONUS_ENTRY_VEC _bonus_entry_vec;
	TOP_SCORE_VEC _top_score_vec;
};

}
#endif
