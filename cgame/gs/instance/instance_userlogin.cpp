#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../world.h"
#include "../player.h"
#include "../player_imp.h"
#include "../config.h"
#include "../userlogin.h"
#include "../usermsg.h"
#include "../clstab.h"
#include "../playertemplate.h"

#include <deque>
#include <db_if.h>
#include "../task/taskman.h"
#include "instance_manager.h"
#include <base64.h>
#include "../shielduser_filter.h"

extern int __allow_login_class_mask;
namespace {
class LoginTask :  public abase::ASmallObject , public GDB::Result
{
	int _uid;
	int _cs_index;
	int _cs_sid;
	void * _auth_data;
	size_t _auth_size;
	bool _isshielduser;
	char _flag;
public:
	LoginTask(int uid,int cs_index, int cs_sid, const void * auth_data , size_t auth_size, bool isshielduser, char flag)
		:_uid(uid),_cs_index(cs_index),_cs_sid(cs_sid),_auth_data(NULL),_auth_size(auth_size),_isshielduser(isshielduser),_flag(flag)
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
	//检测登录控制  
	if(!(__allow_login_class_mask & (1 << (pInfo->cls & 0x3F))))
	{               
		GMSV::SendLoginRe(_cs_index,_uid,_cs_sid,1,_flag);    // login failed
		//此时按照错误来处理
		OnFailed();
		return;
	}

	if(!do_login_check_data(pInfo,data))
	{
		GMSV::SendLoginRe(_cs_index,_uid,_cs_sid,1,_flag);    // login failed
		//此时按照错误来处理
		GLog::log(GLOG_ERR,"用户%d数据异常，无法登录",id);
		OnFailed();
		return; 
	}

	if(pInfo->worldtag != world_manager::GetWorldTag())
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

	//从数据库中取得instance key
	instance_world_manager * pManager = (instance_world_manager*)world_manager::GetInstance();
	instance_hash_key ikey;
	player_var_data::GetInstanceKey(data->var_data.data,data->var_data.size,ikey);


	//进行世界的分配
	int world_index;
	world * pPlane = pManager->GetWorldOnLogin(ikey,world_index);
	if(!pPlane)
	{
		//生成世界失败了，应该是没有足够的副本
		// 防止新手村位置重置res 5->1
		GMSV::SendLoginRe(_cs_index,_uid,_cs_sid,pManager->GetWorldType() != WORLD_TYPE_PARALLEL_WORLD ? 5 : 1,_flag); 
		delete this;
		return;
	}
	gplayer * pPlayer = pPlane->AllocPlayer();
	if(pPlayer == NULL)
	{
		//发送没有物理空间可以容纳Player的信息
		__PRINTF("用户达到物理最大限制值\n");
		// 防止新手村位置重置res 2->1
		GMSV::SendLoginRe(_cs_index,_uid,_cs_sid,pManager->GetWorldType() != WORLD_TYPE_PARALLEL_WORLD ? 2 : 1,_flag); 
		return;
	}
	GLog::log(GLOG_INFO,"用户%d从%d开始登录",_uid,_cs_index);
	pPlayer->cs_sid = _cs_sid;
	pPlayer->cs_index = _cs_index;
	pPlayer->ID.id = _uid;
	pPlayer->ID.type = GM_TYPE_PLAYER;
	pPlayer->login_state = gplayer::WAITING_LOGIN;
	pPlayer->pPiece = NULL;
	
	if(!pPlane->MapPlayer(_uid,pPlane->GetPlayerIndex(pPlayer)))
	{
		//map player 失败，表示在这一瞬间有人加入
		pPlane->FreePlayer(pPlayer);
		mutex_spinunlock(&pPlayer->spinlock);
		GMSV::SendLoginRe(_cs_index,_uid,_cs_sid,4,_flag);       // login failed
		return;
	}
	ASSERT(pPlayer->imp == NULL);
	pPlayer->imp = NULL; 

	userlogin_t user;
	memset(&user,0,sizeof(user));
	user._player = pPlayer;
	user._plane = pPlane;
	user._uid = _uid;
	user._auth_data = _auth_data;
	user._auth_size = _auth_size;

	//设置副本对应的标志
	pManager->SetPlayerWorldIdx(_uid,world_index);

	//检查世界是否和存盘数据一致，如果不一致，则将坐标设置到起点 
	int last_instance_timestamp;
	int last_instance_tag;
	A3DVECTOR last_instance_pos;
	player_var_data::GetLastInstance<0>(data->var_data.data,data->var_data.size,last_instance_tag,last_instance_pos,last_instance_timestamp);

	A3DVECTOR login_pos = A3DVECTOR(pInfo->posx, pInfo->posy, pInfo->posz);

	if(last_instance_tag == world_manager::GetWorldTag())
	{
		if(last_instance_timestamp > 0)
		{
			if(last_instance_timestamp == pPlane->w_create_timestamp)
			{	
				//存盘副本现在依然存在，坐标无需转换 什么都不需要变
			}
			else
			{
				//当前副本已经不存在，使用存盘位置  并更新时间戳
				if(pManager->GetWorldType() != WORLD_TYPE_PARALLEL_WORLD) login_pos = last_instance_pos;
				last_instance_timestamp = pPlane->w_create_timestamp;
			}
		}
		else
		{
			last_instance_tag = -1;
		}
	}
	else
	{
		last_instance_tag = -1;
	}
	//如果没有完成位置的标定，那么玩家的数据将会是错误的，这时该如何？考虑使用配置文件给出的默认入口
	if(last_instance_tag <= 0)
	{
		//试图使用配置文件内可能存在的位置
		if(world_manager::GetSavePoint().tag == world_manager::GetWorldTag())
		{
			//必须tag满足要求才能使用当前的设置
			last_instance_tag = world_manager::GetWorldTag();
			last_instance_pos = world_manager::GetSavePoint().pos;
			last_instance_timestamp = pPlane->w_create_timestamp;

			//将登录位置也改为保存位置
			if(pManager->GetWorldType() != WORLD_TYPE_PARALLEL_WORLD) login_pos = last_instance_pos;
		}
	}

	if(_isshielduser)
		pPlayer->object_state |= gactive_object::STATE_SHIELD_USER; 
	//进行世界的加入工作
	do_player_login(login_pos, pInfo,data,user,_flag);

	if(pPlayer->imp)
	{
		gplayer_imp *pImp =(gplayer_imp*)pPlayer->imp;
		pManager->SetFilterWhenLogin(pImp, NULL);

		//设置副本入口坐标
		pImp->SetLastInstancePos(last_instance_tag, last_instance_pos, last_instance_timestamp);
	}

	if(pPlayer->imp && _isshielduser)
	{
		gplayer_imp *pImp =(gplayer_imp*)pPlayer->imp;
		pImp->_filters.AddFilter(new shielduser_filter(pImp));
	}
	
	//解开玩家的锁
	//等待玩家发来EnterWorld
	mutex_spinunlock(&pPlayer->spinlock);

	//删除自身
	delete this;
}

}


void	instance_user_login(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size, bool isshielduser, char flag)
{
	//副本不允许直接登录
	int world_index;
	if(world_manager::GetInstance()->FindPlayer(uid,world_index))
	{
		//发送已经有人登录的消息
		GMSV::SendLoginRe(cs_index,uid,cs_sid,3,flag);	// login failed
		GLog::log(GLOG_WARNING,"用户%d已经登录(%d,%d)(在世界?d)",uid , cs_index,cs_sid,world_index);
		return ;
	}

	//从数据库取出数据，并进行所有的登录操作
	GDB::get_role(uid, new LoginTask(uid,cs_index,cs_sid,auth_data,auth_size,isshielduser,flag));
}

