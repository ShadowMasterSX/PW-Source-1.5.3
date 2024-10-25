#ifndef __ONLINEGAME_GS_MOBILE_PLAYER_H__
#define __ONLINEGAME_GS_MOBILE_PLAYER_H__

/*
   手机用户的imp
*/

#include "../player_imp.h"

class gplayer_mobile_imp : public gplayer_imp
{
protected:
	int _real_world_tag;		//记录玩家实际的位置
	A3DVECTOR _real_pos;		//记录玩家实际的位置

public:
	gplayer_mobile_imp() : _real_world_tag(1), _real_pos(0.f,0.f,0.f){}
	virtual ~gplayer_mobile_imp(){}
	
public:
	DECLARE_SUBSTANCE(gplayer_mobile_imp);
	virtual int DispatchCommand(int cmd_type, const void * buf,size_t size);
	virtual void SwitchSvr(int dest, const A3DVECTOR & oldpos, const A3DVECTOR &newpos,const instance_key *switch_key){}//禁止切换副本
	virtual const A3DVECTOR & GetLogoutPos(int & world_tag);
	virtual void SaveRealWorldPos(int world_tag, const A3DVECTOR & pos);
	virtual void SendAllData(bool detail_inv, bool detail_equip, bool detail_task);
};

#endif
