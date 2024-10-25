#ifndef __ONLINEGAME_GS_PLAYER_INSTANCE_REENTER_H
#define __ONLINEGAME_GS_PLAYER_INSTANCE_REENTER_H
#include "player.h"

class gplayer_imp;
class player_instance_reenter
{
public:
	player_instance_reenter() : _timeout(0),_instance_tag(0),_world_type(0),_instance_hkey(0) {}
	void LoadFromDB(int tag,int type,instance_hash_key key,int timeout,A3DVECTOR pos);
	void NotifyClient(gplayer_imp* player);
	bool TryReenter(gplayer_imp* player);
protected:
	bool CanReenter();

private:
	int _timeout;		
	int _instance_tag;
	int _world_type;
	instance_hash_key _instance_hkey;
	A3DVECTOR _logout_pos;
};

#endif
