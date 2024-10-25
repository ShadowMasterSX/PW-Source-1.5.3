#ifndef __ONLINEGAME_GS_PLAYER_MISC_IMP_H__
#define __ONLINEGAME_GS_PLAYER_MISC_IMP_H__


extern const A3DVECTOR aExts[USER_CLASS_COUNT*2];
extern const A3DVECTOR aExts2[USER_CLASS_COUNT*2];
class gplayer_imp;
class phase_control  
{
	enum
	{
		STATE_GROUND,
		STATE_JUMP,
		STATE_FALL,
		STATE_FLY,
		STATE_WATER,
	};

	enum
	{
		MAX_JUMP_COUNT = 2,	
	};

	int   state;            //当前状态，可能是 站立（地面？）悬空(跳起/跌落)
	float jump_distance;    //跳起的距离和高度和
	int   jump_time;        //跳起的时间和
	float drop_speed;       //跌落速度控制
	int   jump_count;		//当前跳跃次数
	float max_jump_distance;//由worldmanager中的_world_limit.lowjump控制
	int   max_jump_time;	//由worldmanager中的_world_limit.lowjump控制
	float speed_ctrl_factor;//限制y方向速度的统计因子，意义是当前最近用过的最大合法速度的平方
	int   fall_to_fly_time;	//跌落至飞行状态计时。玩家在跌落过程中开启飞机，刚开始的一段时间(时间长短和网络延迟有关)客户端认为玩家此时在跌落，服务器认为玩家此时在飞行，为防止误判此段时间内服务器只检查玩家的向上速度，并增加错误计数。
	int   fly_to_fall_time;	//飞行至跌落状态计时。玩家在向上飞行过程中关闭飞机，刚开始的一段时间(时间长短和网络延迟有关)客户端认为玩家此时在飞行，服务器认为玩家此时在不在飞行，为防止误判此段时间内服务器允许玩家继续向上飞，并增加错误计数。
public:
	phase_control():state(STATE_GROUND),jump_distance(0.0f),jump_time(0),drop_speed(0.0f),jump_count(0),max_jump_distance(0.0f),max_jump_time(0),speed_ctrl_factor(0.0f),fall_to_fly_time(0),fly_to_fall_time(0){}

	void Swap(phase_control & rhs)
	{
		phase_control tmp(*this);
		*this = rhs;
		rhs = tmp;
	}

	void Load(archive & ar);
	void Save(archive & ar);
	
	void Initialize(gplayer_imp * pImp);
	int PhaseControl(gplayer_imp * pImp, const A3DVECTOR & target, float theight, int mode, int use_time); //每次移动后的状态，本次移动如若成功 会更新自己的状态
	bool CheckLevitate()
	{
		return state == STATE_JUMP || state == STATE_FALL;
	}
};

#endif

