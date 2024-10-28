#ifndef __GNET_WAIT_QUEUE_H
#define __GNET_WAIT_QUEUE_H

#include <list>
#include <map>
#include "itimer.h"
namespace GNET
{

#define	QUEUE_UPDATE_DISTANCE	500000	// 0.5秒心跳一次
#define	MAX_INVITE_USER_PERTICK	10      // 每次心跳最多邀请玩家数
#define MAX_SAMPLING_COUNT		30		// 排队时间采样数量
#define MAX_QUEUE_WAIT_TIME		7200	// 最长显示2小时
#define LOGOUT_INVITE_KEEP_TIME 180		// 下线免登陆排队时间
#define BUSY_CHECK_TICK			40		// 队列紧张时空位检查频率
#define FREE_CHECK_TICK			200		// 队列不紧张时空位检查频率
#define CALC_WAITTIME_TICK		100		// 计算平均等待时间频率
#define REFRESH_TICK			20		// 通知客户端刷新频率	
#define MAX_NOTICE_USER			600		// 最大能获得刷新的人数
#define DEFAULT_NOTICE_USER		100		// 默认能获得刷新的人数

enum WQ_RES
{
	WQ_NOWAIT,
	WQ_BEGWAIT,
	WQ_INQUEUE,
	WQ_MAXUSER,

	WQ_FAIL,
};

struct UserInfoBrief
{
	bool	isvip;
	bool    isgm;
	unsigned int linksid;
	unsigned int localsid;
};

class WaitQueueManager : public IntervalTimer::Observer
{
	enum QUEUE_TYPE
	{
		QUEUE_TYPE_NORMAL  	, // 普通排队队列
		QUEUE_TYPE_VIP 		, // vip 排队队列
		QUEUE_TYPE_INVITE	, // 邀请队列

		QUEUE_TYPE_MAX       
	};

	typedef std::list<int> UserQueue;
	typedef std::list<int> WaittimeList;
	typedef UserQueue::iterator UQ_Iter;

	struct UserState
	{
		int 	state;
		int 	begin_timestamp;
		int     wait_num;
		int 	select_roleid;
		int 	provider_link_id;
		UserQueue::iterator locate;
		UserInfoBrief	info;
	};

	typedef std::map<int, UserState > UserMap;
	typedef UserMap::iterator UM_Iter;
	UserQueue _queues[QUEUE_TYPE_MAX];
	UserMap	  _users;
	WaittimeList _waittime_sampling;
	bool	  _init;
	int		  _average_waittime;
	int 	  _check_tick;	
	int 	  _refresh_tick;
	int       _calc_wait_tick;
public:
	static WaitQueueManager* GetInstance() { static WaitQueueManager instance; return &instance; }
	WaitQueueManager() : _init(false),_average_waittime(0),_check_tick(0),_refresh_tick(0),_calc_wait_tick(0) {}
public:
	bool Initialize();
	bool IsInvited(int uid) 
	{ 
		UserMap::iterator iter = _users.find(uid);
		if(iter == _users.end()) return false;
		return  iter->second.state == QUEUE_TYPE_INVITE;
	}
	void OnBecomeVip(int uid);
	bool OnCancelWait(int uid);
	void OnLogout(int uid);
	int  OnPlayerLogin(int uid,int rid,int lid);

	bool Update();
	void SendQueue(int uid,int pos = 0);
	void SendQueueMsgAll();
private:
	void IniviteUser(int uid);
	void EraseUser(int uid);
	void CalcAverageWaittime();
	void CheckInviteUser();
	void ContinueLogin(int uid,int rid,int linkid);
	void PushQueue(UserState& user,int state,int uid); 
	void PopQueue(UserState& user); 
};

}

#endif        
