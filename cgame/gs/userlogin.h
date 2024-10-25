#ifndef __ONLINEGAME_GS_USERLOGIN_H__
#define __ONLINEGAME_GS_USERLOGIN_H__

#include "object.h"
#include <threadpool.h>
#include <amemobj.h>
#include <db_if.h>

void	user_login(int cs_index,int sid,int uid, const void * auth_data, size_t auth_size, bool isshielduser, char flag);
void 	user_enter_world(int cs_idnex,int sid,int uid);

struct gplayer;
void 	user_save_data(gplayer * pPlayer,GDB::Result * callback = NULL, int priority = 0,int mask = DBMASK_PUT_ALL);

class world;
struct userlogin_t
{
	gplayer * _player;
	world * _plane;
	int _uid;
	void * _auth_data;
	size_t _auth_size;
};

void do_player_login(const A3DVECTOR & ppos,const GDB::base_info * pInfo, const GDB::vecdata * data,const userlogin_t &user, char flag);

bool do_login_check_data(const GDB::base_info * pInfo, const GDB::vecdata * data);

#endif

