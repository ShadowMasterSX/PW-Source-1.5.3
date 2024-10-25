#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arandomgen.h>

#include <common/protocol.h>
#include "world.h"
#include "player_imp.h"
#include "usermsg.h" 
#include "clstab.h"
#include "actsession.h"
#include "userlogin.h"
#include "playertemplate.h"
#include "serviceprovider.h"
#include <common/protocol_imp.h>
#include "trade.h"
#include "task/taskman.h"
#include "sitdown_filter.h"
#include "playerstall.h"
#include "gm_player.h"

DEFINE_SUBSTANCE(gm_dispatcher,gplayer_dispatcher, CLS_GM_DISPATCHER)

void 
gm_dispatcher::enter_slice(slice *  pPiece,const A3DVECTOR &pos)
{
	if(!_gm_invisible) return gplayer_dispatcher::enter_slice(pPiece,pos);
		
	//取得新区域数据
	get_slice_info(pPiece,_nw,_mw,_pw);
}

void 
gm_dispatcher::leave_slice(slice * pPiece,const A3DVECTOR &pos)
{
	if(!_gm_invisible) return gplayer_dispatcher::leave_slice(pPiece,pos);
	
	pPiece->Lock();
	gather_slice_object(pPiece,_leave_list);
	pPiece->Unlock();
}

void
gm_dispatcher::move(const A3DVECTOR & target, int cost_time,int speed,unsigned char move_mode)
{
	if(_gm_invisible) return;
	gplayer_dispatcher::move(target,cost_time,speed,move_mode);
}

void
gm_dispatcher::stop_move(const A3DVECTOR & target, unsigned short speed ,unsigned char dir,unsigned char move_mode)
{
	if(_gm_invisible) return;
	gplayer_dispatcher::stop_move(target,speed,dir,move_mode);
}

void
gm_dispatcher::enter_world()
{
	if(!_gm_invisible) return gplayer_dispatcher::enter_world();
	
	//取得进入一个区域应当取得数据
	enter_region();
}

void
gm_dispatcher::leave_world()
{
	if(!_gm_invisible) return gplayer_dispatcher::leave_world();
}

void
gm_dispatcher::disappear()
{
	if(!_gm_invisible) return gplayer_dispatcher::disappear();
}

void 
gm_dispatcher::notify_move(const A3DVECTOR &oldpos, const A3DVECTOR & newpos)
{
	if(_gm_invisible) return;
	gplayer_dispatcher::notify_move(oldpos,newpos);
}

void 
gm_dispatcher::appear()
{
	if(!_gm_invisible) return gplayer_dispatcher::appear();
}

void 
gm_dispatcher::on_dec_invisible(int prev_invi_degree, int cur_invi_degree)
{
	if(!_gm_invisible) return gplayer_dispatcher::on_dec_invisible(prev_invi_degree,cur_invi_degree);
}

void 
gm_dispatcher::resurrect(int level)
{
	if(!_gm_invisible) return gplayer_dispatcher::resurrect(level);
	//只发给自己	
	_tbuf.clear();
	using namespace S2C;
	gplayer * pPlayer = (gplayer*)_imp->_parent;
	CMD::Make<CMD::player_revival>::From(_tbuf,pPlayer,level);
	send_ls_msg(pPlayer, _tbuf);
}

enum{MIN_SEND_COUNT = 128};

void 
gm_dispatcher::enter_region()
{
	if(!_gm_invisible) return gplayer_dispatcher::enter_region();
		
	world * pPlane = _imp->_plane;
	slice * pPiece = _imp->_parent->pPiece;
	get_slice_info(pPiece,_nw,_mw,_pw);

	int i;
	int total = pPlane->w_far_vision;
	int index = pPlane->GetGrid().GetSliceIndex(pPiece);
	int slice_x,slice_z;
	pPlane->GetGrid().Index2Pos(index,slice_x,slice_z);
	for(i = 0; i <total; i ++)
	{
		world::off_node_t &node = pPlane->w_off_list[i]; 
		int nx = slice_x + node.x_off; 
		int nz = slice_z + node.z_off; 
		if(nx < 0 || nz < 0 || nx >= pPlane->GetGrid().reg_column || nz >= pPlane->GetGrid().reg_row) continue;
		slice * pNewPiece = pPiece + node.idx_off;
		if(i <= pPlane->w_true_vision)
		{
			get_slice_info(pNewPiece,_nw,_mw,_pw);
		}
		else
		{
			get_slice_info(pNewPiece,_nw,_mw,_pw);
		}
		wrapper_test<MIN_SEND_COUNT>(_pw,_header,S2C::PLAYER_INFO_1_LIST);
		wrapper_test<MIN_SEND_COUNT>(_mw,_header,S2C::MATTER_INFO_LIST);
		wrapper_test<MIN_SEND_COUNT>(_nw,_header,S2C::NPC_INFO_LIST);
	}
	return ;
}
