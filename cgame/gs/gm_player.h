#ifndef __ONLINE_GAME_GS_GM_PLAYER_H__
#define __ONLINE_GAME_GS_GM_PLAYER_H__
#include "player_imp.h"

//GM在未通过控制台进入隐身时与正常玩家相同,但在通过控制台切换为隐身状态时则拥有完全视野

class gm_dispatcher : public  gplayer_dispatcher
{
	bool _gm_invisible;
public:
	DECLARE_SUBSTANCE(gm_dispatcher);
public:
	bool Save(archive & ar)
	{
		gplayer_dispatcher::Save(ar);
		ar << _gm_invisible;
		return true;
	}
	bool Load(archive & ar)
	{
		gplayer_dispatcher::Load(ar);
		ar >> _gm_invisible;
		return true;
	}
	virtual void set_gm_invisible(bool invisible ) {_gm_invisible = invisible;}
	virtual bool is_gm_invisible() {return _gm_invisible;}
	gm_dispatcher():_gm_invisible(false){}
	virtual void enter_slice(slice * ,const A3DVECTOR &pos);
	virtual void leave_slice(slice * ,const A3DVECTOR &pos);
	virtual void enter_world();	//cache
	virtual void leave_world();
	virtual void move(const A3DVECTOR & target, int cost_time,int speed,unsigned char move_mode);
	virtual void stop_move(const A3DVECTOR & target, unsigned short speed,unsigned char dir,unsigned char move_mmode);
	virtual void disappear();
	virtual void notify_move(const A3DVECTOR &oldpos, const A3DVECTOR & newpos);
	virtual void appear();
	virtual void on_dec_invisible(int prev_invi_degree, int cur_invi_degree);
	virtual void resurrect(int);
	virtual void LoadFrom(gplayer_dispatcher * rhs)		//从原有的dispatcher中取得数据
	{
		gplayer_dispatcher::LoadFrom(rhs);
		gplayer * pPlayer = (gplayer*) _imp->_parent;
		_gm_invisible = pPlayer->gm_invisible;
	}
	
protected:
	virtual void enter_region();
};

#endif
