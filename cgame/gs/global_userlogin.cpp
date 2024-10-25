#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "world.h"
#include "player.h"
#include "player_imp.h"
#include "config.h"
#include "userlogin.h"
#include "usermsg.h"
#include "clstab.h"
#include "playertemplate.h"

#include <deque>
#include <db_if.h>
#include "task/taskman.h"
#include <base64.h>
#include "shielduser_filter.h"

extern int __allow_login_class_mask;
namespace 
{
class LoginTask :  public abase::ASmallObject , public GDB::Result
{
	gplayer * _player;
	world * _plane;
	int _uid;
	int _cs_index;
	int _cs_sid;
	void * _auth_data;
	size_t _auth_size;
	bool _isshielduser;
	char _flag;
public:
	LoginTask(world * pPlane,gplayer * pPlayer,int uid,const void * auth_data , size_t auth_size, bool isshielduser, char flag)
		:_player(pPlayer),_plane(pPlane),_uid(uid),_cs_index(pPlayer->cs_index),_cs_sid(pPlayer->cs_sid),_auth_data(NULL),_auth_size(auth_size),_isshielduser(isshielduser),_flag(flag)
		{
			if(auth_size)
			{
				_auth_data = abase::fastalloc(auth_size);
				memcpy(_auth_data,auth_data,auth_size);
			}
		}
	~LoginTask()
	{
		if(_auth_data)
		{
			abase::fastfree(_auth_data,_auth_size);
		}
	}

	void Failed()
	{
		GMSV::SendLoginRe(_cs_index,_uid,_cs_sid,-1,_flag);	// login failed
		//对申请的player对象进行释放
		spin_autolock alock(_player->spinlock);
		ASSERT(_player->login_state == gplayer::WAITING_LOGIN && _uid == _player->ID.id);
		if(_player->is_waitting_login() && _uid == _player->ID.id) 
		{
			_plane->UnmapPlayer(_uid);
			_plane->FreePlayer(_player);
		}
		delete this;
	}
public:
	virtual void OnTimeOut()
	{
		GLog::log(GLOG_ERR,"用户%d从数据库取得数据超时",_uid);
		Failed();
	}
	
	virtual void OnFailed()
	{
		GLog::log(GLOG_ERR,"用户%d从数据库取得数据失败",_uid);
		Failed();
	}
	virtual void OnGetRole(int id,const GDB::base_info * pInfo, const GDB::vecdata * data,const GNET::GRoleDetail* pRole);
};

void 
LoginTask::OnGetRole(int id,const GDB::base_info * pInfo, const GDB::vecdata * data, const GNET::GRoleDetail * pRole)
{
	//检测登录控制  只有被允许的职业才能够登入
	if(!(__allow_login_class_mask & (1 << (pInfo->cls & 0x3F))))
	{
		GMSV::SendLoginRe(_cs_index,_uid,_cs_sid,1,_flag);	// login failed
		//此时按照错误来处理
		GLog::log(GLOG_ERR,"用户%d由于职业%d被禁止登入，登录失败",id,pInfo->cls & 0x3F);
		OnFailed();
		return;
	}

	if(!do_login_check_data(pInfo,data))
	{
		GMSV::SendLoginRe(_cs_index,_uid,_cs_sid,1,_flag);	// login failed
		//此时按照错误来处理
		GLog::log(GLOG_ERR,"用户%d数据异常，无法登录",id);
		OnFailed();
		return;
	}

	//手机用户服务器不检查玩家位置
	if(!world_manager::GetInstance()->IsMobileWorld() && pInfo->worldtag != world_manager::GetWorldTag())
	{
		GMSV::SendLoginRe(_cs_index,_uid,_cs_sid,1,_flag);	// login failed
		//此时按照错误来处理
		GLog::log(GLOG_ERR,"用户%d的worldtag(%d)与当前地图不匹配(%d)",id,pInfo->worldtag,world_manager::GetWorldTag());
		OnFailed();
		return;
	}
	
	//从数据库端取得数据

	char name_base64[64] ="未知";
	if(data->user_name.data)
	{
		size_t name_len = data->user_name.size;
		if(name_len > 32) name_len = 32;
		base64_encode((unsigned char*)(data->user_name.data),name_len,name_base64);
	}
	
	GLog::log(GLOG_INFO,"用户%d从数据库取得数据，职业%d,级别%d 名字'%s'",_uid,pInfo->cls,pInfo->level,name_base64);
	spin_autolock alock(_player->spinlock);
	if(!_player->is_waitting_login() || _uid != _player->ID.id) 
	{	
		//已经不是登录状态了 这是一个奇怪的错误
		ASSERT(false);
		GMSV::SendLoginRe(_cs_index,_uid,_cs_sid,1,_flag);	// login failed
		delete this;
		return;
	}

	if(_player->b_disconnect)
	{
		GMSV::SendDisconnect(_cs_index, _uid, _cs_sid,0);
		_plane->UnmapPlayer(_player->ID.id);
		_plane->FreePlayer(_player);
		delete this;
		return;
	}

	userlogin_t user;
	memset(&user,0,sizeof(user));
	user._player = _player;
	user._plane = _plane;
	user._uid = _uid;
	user._auth_data = _auth_data;
	user._auth_size = _auth_size;

	if(_isshielduser)
		_player->object_state |= gactive_object::STATE_SHIELD_USER; 
	
	do_player_login(A3DVECTOR(pInfo->posx, pInfo->posy, pInfo->posz), pInfo,data,user,_flag);

	if(_player->imp)
	{
		gplayer_imp *pImp =(gplayer_imp*)_player->imp;
		world_manager::GetInstance()->SetFilterWhenLogin(pImp);
	}

	if(_player->imp && _isshielduser)
	{
		gplayer_imp *pImp =(gplayer_imp*)_player->imp;
		pImp->_filters.AddFilter(new shielduser_filter(pImp));
	}

	//删除自身
	delete this;
}
}

void	global_user_login(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size, bool isshielduser, char flag)
{
	//应该先查询是否已经在本服务器上了
	world * pPlane = world_manager::GetInstance()->GetWorldByIndex(0);
	int rindex;
	if((rindex = pPlane->FindPlayer(uid)) >=0)
	{
		//发送已经有人登录的消息
		GMSV::SendLoginRe(cs_index,uid,cs_sid,3,flag);	// login failed
		GLog::log(GLOG_WARNING,"用户%d已经登录(%d,%d)(%d)",uid,cs_index,cs_sid,pPlane->GetPlayerByIndex(rindex)->login_state);
		return ;
	}
	gplayer * pPlayer = pPlane->AllocPlayer();
	if(pPlayer == NULL)
	{
		//发送没有物理空间可以容纳Player的信息
		GMSV::SendLoginRe(cs_index,uid,cs_sid,2,flag);	// login failed
		GLog::log(GLOG_WARNING,"用户达到物理限制最大值 uid:%d",uid);
		return;
	}
	GLog::log(GLOG_INFO,"用户%d从%d开始登录",uid,cs_index);
	pPlayer->cs_sid = cs_sid;
	pPlayer->cs_index = cs_index;
	pPlayer->ID.id = uid;
	pPlayer->ID.type = GM_TYPE_PLAYER; 
	pPlayer->login_state = gplayer::WAITING_LOGIN; 
	pPlayer->pPiece = NULL;
	if(!pPlane->MapPlayer(uid,pPlane->GetPlayerIndex(pPlayer)))
	{
		//map player 失败，表示在这一瞬间有人加入
		pPlane->FreePlayer(pPlayer);
		mutex_spinunlock(&pPlayer->spinlock);
		GMSV::SendLoginRe(cs_index,uid,cs_sid,4,flag);	// login failed
		return;
	}
	
	ASSERT(pPlayer->imp == NULL);
	pPlayer->imp = NULL; 

	mutex_spinunlock(&pPlayer->spinlock);

	//从数据库系统取出数据,并完成login操作
	GDB::get_role(uid, new LoginTask(pPlane,pPlayer,uid,auth_data,auth_size,isshielduser,flag));
}


