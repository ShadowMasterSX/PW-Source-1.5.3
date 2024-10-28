#include "waitqueue.h"
#include "waitqueuestatenotify.hpp"
#include "playerlogin.hpp"
#include "playerlogin_re.hpp"
#include "gproviderserver.hpp"
#include "mapforbid.h"
#include "gamedbclient.hpp"
#include "mappasswd.h"
#include "dbclearconsumable.hrp"

#define IS_VALID_STATE(s) ((unsigned int)s < QUEUE_TYPE_MAX)
#define FIND_USER(uid) UserMap::iterator iter = _users.find(uid); if(iter == _users.end()) return; UserState& user = iter->second;

bool WaitQueueManager::Initialize()
{
	if(_init) return false;
	_init = true;

	IntervalTimer::Attach(this, QUEUE_UPDATE_DISTANCE/IntervalTimer::Resolution());
	LOG_TRACE( "WaitQueueManager Init Successfully!!\n");
	return true;
}

void WaitQueueManager::PushQueue(UserState& user,int state,int uid) 
{
	user.state = state;
	user.locate = _queues[state].insert(_queues[state].end(),uid);
}

void WaitQueueManager::PopQueue(UserState& user)
{
	if(IS_VALID_STATE(user.state))
	{
		_queues[user.state].erase(user.locate);
		user.state = QUEUE_TYPE_MAX; 
	}
}

int WaitQueueManager::OnPlayerLogin(int uid,int rid,int lid)
{
	if(!_init) 
		return WQ_NOWAIT;
	
	LOG_TRACE( "WaitQueueManager: Login User=%d Role=%d Linkid=%d\n", uid,rid,lid);
	
	UserMap::iterator iter = _users.find(uid);
	if(iter != _users.end())
	{
		if(iter->second.state == QUEUE_TYPE_INVITE)
			return WQ_NOWAIT;
		if(!UserContainer::GetInstance().FillUserBrief(uid,iter->second.info))
		{
			EraseUser(uid);
			return WQ_FAIL;
		}

		iter->second.select_roleid = rid;
		iter->second.provider_link_id = lid;
		SendQueue(uid);
		return WQ_INQUEUE; 
	}

	if(_queues[QUEUE_TYPE_NORMAL].size()+_queues[QUEUE_TYPE_VIP].size() > UserContainer::GetInstance().GetWaitLimit())	
	{
		// 理论上不会出现,userlogin已经挡住了
		return WQ_MAXUSER;
	}

	UserState newuser;
	if(!UserContainer::GetInstance().FillUserBrief(uid,newuser.info))
	{
		return WQ_FAIL;
	}

	if(newuser.info.isgm 
	|| (_queues[QUEUE_TYPE_NORMAL].empty() && _queues[QUEUE_TYPE_VIP].empty() && _queues[QUEUE_TYPE_INVITE].size() < UserContainer::GetInstance().GetInGameLimit()))
	{
		// 直接进入
		newuser.select_roleid = rid;
		newuser.provider_link_id = lid;
		PushQueue(newuser,QUEUE_TYPE_INVITE,uid);
		newuser.begin_timestamp = 0;

		_users[uid] = newuser;
		return WQ_NOWAIT;
	}

	if(!newuser.info.isvip && (_queues[QUEUE_TYPE_NORMAL].size()+1)*2 > UserContainer::GetInstance().GetWaitLimit())
	{
		// 给vip队列预留位置
		return WQ_MAXUSER;
	}

	newuser.select_roleid = rid;
	newuser.provider_link_id = lid;
	newuser.begin_timestamp = time(NULL); 
	PushQueue(newuser,newuser.info.isvip ? QUEUE_TYPE_VIP : QUEUE_TYPE_NORMAL,uid);
	newuser.wait_num = newuser.info.isvip ? _queues[QUEUE_TYPE_VIP].size() : 
		_queues[QUEUE_TYPE_VIP].size() + _queues[QUEUE_TYPE_NORMAL].size();

	_users[uid] = newuser;
	SendQueue(uid,_queues[newuser.state].size());

	return WQ_BEGWAIT;
}

void WaitQueueManager::OnBecomeVip(int uid)
{
	if(!_init) return;

	LOG_TRACE( "WaitQueueManager: BecomeVip User=%d \n", uid);
	FIND_USER(uid);
	if(user.state != QUEUE_TYPE_NORMAL)  return;
	PopQueue(user);
	PushQueue(user,QUEUE_TYPE_VIP,uid);
}

bool WaitQueueManager::OnCancelWait(int uid)
{
	if(!_init) return false;

	LOG_TRACE( "WaitQueueManager: Cancel wait User=%d \n", uid);
	UserMap::iterator iter = _users.find(uid); if(iter == _users.end()) return false;
	UserState& user = iter->second;
	if(!IS_VALID_STATE(user.state) || user.state == QUEUE_TYPE_INVITE)  return false;
	EraseUser(uid);
	return true;
}

void WaitQueueManager::OnLogout(int uid)
{
	if(!_init) return;

	FIND_USER(uid);
	if(user.state == QUEUE_TYPE_INVITE)
	{
		user.begin_timestamp = time(NULL);
	}
	else
	{
		EraseUser(uid);
	}
}

void WaitQueueManager::EraseUser(int uid)
{
	FIND_USER(uid);
	PopQueue(user);
	_users.erase(uid);
}

void WaitQueueManager::IniviteUser(int uid)
{
	FIND_USER(uid);

	LOG_TRACE( "WaitQueueManager: Inivite User=%d \n", uid);
	if(user.state == QUEUE_TYPE_INVITE) return;
	PopQueue(user);
	PushQueue(user,QUEUE_TYPE_INVITE,uid);

	int waittime = time(NULL) - user.begin_timestamp;

	_waittime_sampling.push_back(user.wait_num ? (waittime/user.wait_num) : waittime);	

	ContinueLogin(uid,user.select_roleid,user.provider_link_id);
}

void WaitQueueManager::SendQueueMsgAll()
{
	static unsigned char counter = 0;
	int max = (++counter%12 == 0) ? MAX_NOTICE_USER : DEFAULT_NOTICE_USER;	
	
	UQ_Iter iter = _queues[QUEUE_TYPE_NORMAL].begin();
	UQ_Iter iend = _queues[QUEUE_TYPE_NORMAL].end();

	int vipqueue_size = _queues[QUEUE_TYPE_VIP].size();
	WaitQueueStateNotify p(0,0,vipqueue_size,0,0);
	for(int i = 1;iter != iend && i <= max; ++ i, ++iter)
	{
		UserMap::iterator uiter = _users.find(*iter);
		if(uiter == _users.end()) continue;
		UserState& user = uiter->second;
		p.localsid = user.info.localsid;
		p.cur_queue_pos = i;
		p.waittime = _average_waittime * (i + vipqueue_size);
		GDeliveryServer::GetInstance()->Send(user.info.linksid,p);
	}
	
	iter = _queues[QUEUE_TYPE_VIP].begin();
	iend = _queues[QUEUE_TYPE_VIP].end();

	p.vip = 1;	
	for(int i = 1;iter != iend && i <= max; ++ i, ++iter)
	{
		UserMap::iterator uiter = _users.find(*iter);
		if(uiter == _users.end()) continue;
		UserState& user = uiter->second;
		p.localsid = user.info.localsid;
		p.cur_queue_pos = i;
		p.waittime = _average_waittime * i;
		GDeliveryServer::GetInstance()->Send(user.info.linksid,p);
	}
	_refresh_tick = 0;
}

void WaitQueueManager::SendQueue(int uid,int pos)
{
	FIND_USER(uid);
	if(user.state == QUEUE_TYPE_INVITE || !IS_VALID_STATE(user.state) ) return;
	if(!pos) // 找排在前面的有多少人
	{
		pos = 1;
		UQ_Iter iter = _queues[user.state].begin();
		UQ_Iter iend = _queues[user.state].end();
		for(;iter != iend; ++iter,++pos)
		{
			if(*iter == uid) break;
		}
	}
	int vpos = _queues[QUEUE_TYPE_VIP].size();
	int waittime = user.info.isvip ? pos*_average_waittime : (vpos+pos)*_average_waittime;
	if(waittime > MAX_QUEUE_WAIT_TIME) waittime = MAX_QUEUE_WAIT_TIME;

	GDeliveryServer::GetInstance()->Send(user.info.linksid,
			WaitQueueStateNotify(user.info.localsid,
			user.info.isvip ? 1 : 0, vpos,pos, waittime));
}

void WaitQueueManager::ContinueLogin(int userid,int roleid,int linkid)
{
	bool blSuccess = false;
	
	UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);

	if(!pinfo) return;

	LOG_TRACE( "WaitQueueManager: User=%d Role=%d ContinueLogin\n", userid,roleid);

	bool usbbind = Passwd::GetInstance().IsUsbUser(userid); 	
	int gs_id = 0;
	if (!pinfo->is_phone)	//如果不是使用手机登录，走正常登陆流程
	{
		gs_id = GProviderServer::FindGameServer( *(GProviderServer::point_t*)(&pinfo->role_pos[roleid % MAX_ROLE_COUNT]),
				pinfo->worldtag[roleid % MAX_ROLE_COUNT] );
		if (GProviderServer::GetInstance()->IsPhoneGS(gs_id))	//电脑登陆不允许进入手机gs
			gs_id = _GAMESERVER_ID_INVALID;
		LOG_TRACE("PlayerLogin userid %d roleid %d usbbind %d gs_id %d", userid, roleid, usbbind, gs_id);
	}
	else	//使用手机登录服务器，分配到手机gs
	{
		if (!GProviderServer::GetInstance()->GetLessPhoneGS(gs_id))
			gs_id = _GAMESERVER_ID_INVALID;
		LOG_TRACE("PlayerLogin on phone. userid %d roleid %d usbbind %d gs_id=%d", userid, roleid, usbbind, gs_id);
	}

	if (gs_id!=_GAMESERVER_ID_INVALID)
	{
		if(GProviderServer::GetInstance()->DispatchProtocol(gs_id,
					PlayerLogin(roleid,linkid,pinfo->localsid,
					pinfo->privileges,usbbind,0)))
		{
			DBClearConsumable::Check360User(pinfo);
			pinfo->gameid = gs_id;
			pinfo->status = _STATUS_SELECTROLE;
			pinfo->linkid = linkid;
			ForbiddenUsers::GetInstance().Push(userid,roleid,pinfo->status);
			UserContainer::GetInstance().RoleLogin(pinfo, roleid);
			GProviderServer::GetInstance()->AddPhoneGSPlayerNum(gs_id);	//会在函数里边检测是否手机gs
			blSuccess=true;
		}
		else
			Log::log(LOG_ERR,"PlayerLogin send wrong! userid %d roleid %d usbbind %d gs_id=%d", userid, roleid, usbbind, gs_id);
	}
	else
		Log::log(LOG_ERR,"PlayerLogin wrong! userid %d roleid %d usbbind %d gs_id=%d world_id=%d", userid, roleid, usbbind, gs_id, pinfo->worldtag[roleid % MAX_ROLE_COUNT]);
	if (!blSuccess)
		GDeliveryServer::GetInstance()->Send(pinfo->linksid,
				PlayerLogin_Re(ERR_LOGINFAIL,roleid,_PROVIDER_ID_INVALID,pinfo->localsid, 0));

}

void WaitQueueManager::CalcAverageWaittime()
{
	if(_waittime_sampling.empty()) return;

	int step = _waittime_sampling.size()-MAX_SAMPLING_COUNT;
	WaittimeList::iterator iter,iend;

	if(step > 0)
	{
		iend = iter = _waittime_sampling.begin();
		while(--step >= 0) ++iend;
		_waittime_sampling.erase(iter,iend);
	}

	iter = _waittime_sampling.begin();
	iend = _waittime_sampling.end();
	int total_time = 0;
	for(;iter != iend; ++iter)
	{
		total_time += *iter;
	}
	_average_waittime = total_time/_waittime_sampling.size();
	_calc_wait_tick = 0;
}

void WaitQueueManager::CheckInviteUser()
{
	int now = time(NULL);
	UM_Iter iter = _users.begin();
	UM_Iter iend = _users.end();
	int free_count = 0;
	for(;iter != iend;)
	{
		int uid = iter->first;
		UserState& user = iter->second;
		++iter;
		UserInfo* pinfo = UserContainer::GetInstance().FindUser(uid);
		if(!pinfo && (now - user.begin_timestamp >= LOGOUT_INVITE_KEEP_TIME))
		{			
			PopQueue(user);
			_users.erase(uid);	
			++free_count;
		}
	}
	_check_tick = 0;
	LOG_TRACE( "WaitQueueManager Free Logout User Count=%d.\n", free_count);
}

bool WaitQueueManager::Update()
{
	if(!_init) return false;

	int invite_count = UserContainer::GetInstance().GetInGameLimit() - _queues[QUEUE_TYPE_INVITE].size(); 	

	if(invite_count > 0)
	{
		if(invite_count > MAX_INVITE_USER_PERTICK) invite_count = MAX_INVITE_USER_PERTICK; 

		UQ_Iter iter = _queues[QUEUE_TYPE_VIP].begin();
		UQ_Iter iend = _queues[QUEUE_TYPE_VIP].end();
		for(;iter != iend && invite_count > 0; --invite_count)
		{
			IniviteUser(*(iter++));
		}
		iter = _queues[QUEUE_TYPE_NORMAL].begin();
		iend = _queues[QUEUE_TYPE_NORMAL].end();
		for(;iter != iend && invite_count > 0; --invite_count)
		{
			IniviteUser(*(iter++));
		}
	}

	++_check_tick;
	if((invite_count && _check_tick >= FREE_CHECK_TICK) || _check_tick >= BUSY_CHECK_TICK) 
		CheckInviteUser();
	
	if(++_calc_wait_tick >= CALC_WAITTIME_TICK)
		CalcAverageWaittime();

	if(++_refresh_tick >= REFRESH_TICK)
		SendQueueMsgAll();

	return true;
}

