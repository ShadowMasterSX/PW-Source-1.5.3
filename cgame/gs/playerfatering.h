#ifndef __ONLINEGAME_GS_PLAYER_FATE_RING_H__
#define __ONLINEGAME_GS_PLAYER_FATE_RING_H__

#include "vector.h"
#include "common/packetwrapper.h"
#include "config.h"

#define PLAYER_FATE_RING_TOP_LEVEL_OPEN	7	//目前开放的最高等级
#define PLAYER_FATE_RING_UPDATE_WEEKDAY 6 //每周的哪一天更新元魂吸取次数

class gplayer_imp;
class fatering_property;

// 每个命轮的属性
class fate_ring_enum
{
public:
	int level;	//命轮等级
	int power;	//命轮能量
	
	fate_ring_enum()
	{
		level = 0;
		power = 0;
	}

	bool Enhance(gplayer_imp * imp, int type) const;
	bool Impair(gplayer_imp * imp, int type) const;
};

/*
 * 这个类用来保存玩家的命轮属性
 */
class player_fatering
{
public:
	player_fatering(gplayer_imp* p):next_update_time(0),gain_times(0),_owner(p)
	{
		fate_rings.reserve(PLAYER_FATE_RING_TOTAL);
		
		for (size_t i = 0; i < PLAYER_FATE_RING_TOTAL; i++)
		{
			fate_rings.push_back(fate_ring_enum());
		}
	}

	inline int GetGainTimes() const {return gain_times;}
	inline void SetGainTimes(int gt) {gain_times = gt;}
	inline void AddGainTimes() {gain_times++;}

	bool Save(archive & ar);
	bool Load(archive & ar);
	bool InitFromDB(archive & ar);
	bool SaveToDB(archive & ar);
	void Swap(player_fatering & rhs);

	void OnHeartbeat(int curtime);

	//发送刷新数据给客户端
	void NotifyClientRefresh() const;

	//玩家使用元魂增加命轮经验
	bool OnUseSoul(int type, int level, int power);
	
	//增加命轮经验
	bool AddFateRingPower(int type, int power);

	//初始化的时候给玩家增加属性
	void EnhanceAll();

private:
	
	//上线和心跳时调用，检测刷新吸取次数
	bool CheckRefresh(int curtime);

	typedef abase::vector<fate_ring_enum, abase::fast_alloc<> > FATE_RING_VEC;
	FATE_RING_VEC fate_rings;
	int next_update_time;	//下次更新的时间
	int gain_times;		//本周已经采集元魂的次数
	gplayer_imp * const _owner;	//归属玩家的指针
};

#endif
