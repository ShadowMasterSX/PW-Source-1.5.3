#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "world.h"
#include "player_imp.h"
#include "playertemplate.h"

template <typename VEC,typename F>
void EraseVectorElement(VEC& vec,F func)
{
	typename VEC::iterator iter = vec.begin();
	typename VEC::iterator iend = vec.end();

	for(;iter != iend; ++iter)
	{
		if(func(iter))
		{
			vec.erase_noorder(iter);
			return;
		}
	}
}

struct CheckIdFunctor
{
	TITLE_ID	destid;
	CheckIdFunctor(TITLE_ID id) : destid(id) {}
	bool operator()(player_title::TITLE_ID_VEC::iterator it)
	{
		return *it == destid;	
	}
};

struct CheckExpireFunctor
{
	TITLE_ID	destid;
	CheckExpireFunctor(TITLE_ID id) : destid(id) {}
	bool operator()(player_title::TITLE_EXPIRE_VEC::iterator it)
	{
		return (*it).first == destid;	
	}
};


bool player_title::Save(archive & ar)
{
	ar << _current_title;

	ar << _delivered_titles.size();
	if(!_delivered_titles.empty())  ar.push_back(_delivered_titles.begin(),_delivered_titles.size()*sizeof(TITLE_ID));

	ar << _expire_titles.size();

	// 因为数据对齐问题不能用	ar.push_back(_expire_titles.begin(),_expire_titles.size()*sizeof(TITLE_EXPIRE));
	for(size_t i = 0; i < _expire_titles.size(); ++i)
	{
		ar << _expire_titles[i].first;
		ar << _expire_titles[i].second;
	}

	return true;
}

bool player_title::Load(archive & ar)
{
	ar >> _current_title;

	size_t dvsize,evsize;

	ar >> dvsize;
	if(dvsize)
	{
		if(dvsize < 65535)
		{
			_delivered_titles.reserve(dvsize);
			
			TITLE_ID tid;
			for(size_t i = 0; i < dvsize; ++i)
			{
				ar >> tid;
				_delivered_titles.push_back(tid);
			}
		}
	}

	ar >> evsize;
	if(evsize)
	{
		if(evsize < 65535)
		{
			_expire_titles.reserve(evsize);
			TITLE_ID tid;
			int expire;
			for(size_t i = 0; i < evsize; ++i)
			{
				ar >> tid;
				ar >> expire;
				_expire_titles.push_back(abase::pair<TITLE_ID,int>(tid,expire));
			}
		}
	}
	
	return true;
}

void player_title::Swap(player_title & rhs)
{
	int tmp = _current_title;
	_current_title = rhs._current_title;
	rhs._current_title = tmp;
	_delivered_titles.swap(rhs._delivered_titles);
	_expire_titles.swap(rhs._expire_titles);

	GLog::log(LOG_DEBUG,"title swap\n");
}

bool player_title::InitFromDB(archive & ar)
{
	if(0 == ar.size()) // 针对未登录过的老角色
	{
		_current_title = 0;
		_delivered_titles.clear();
		_expire_titles.clear();
		return true;
	}
	
	try{
		ar >> _current_title;

		size_t dvsize,evsize;

		ar >> dvsize;
		if(dvsize)
		{
			if(dvsize < 65535)
			{
				_delivered_titles.reserve(dvsize);
				ASSERT(_delivered_titles.capacity() == dvsize/* 分配已获取称号列表失败*/);
				
				TITLE_ID tid;
				for(size_t i = 0; i < dvsize; ++i)
				{
					ar >> tid;
					_delivered_titles.push_back(tid);
				}
			}
			else
			{
				GLog::log(LOG_ERR,"roleid:%d unmarshal title delivery size%d fail \n",_owner->_parent->ID.id,dvsize);
			}
		}

		ar >> evsize;
		if(evsize)
		{
			if(evsize < 65535)
			{
				_expire_titles.reserve(evsize);
				ASSERT(_expire_titles.capacity() == evsize /* 分配待过期称号列表失败*/);
				TITLE_ID tid;
				int expire;
				for(size_t i = 0; i < evsize; ++i)
				{
					ar >> tid;
					ar >> expire;
					_expire_titles.push_back(abase::pair<TITLE_ID,int>(tid,expire));
				}
			}
			else
			{
				GLog::log(LOG_ERR,"roleid:%d unmarshal title expire size%d fail \n",_owner->_parent->ID.id,evsize);
			}
		}
	}
	catch(...)
	{
		_current_title = 0;
		_delivered_titles.clear();
		_expire_titles.clear();
		GLog::log(LOG_ERR,"roleid:%d unmarshal title fail \n",_owner->_parent->ID.id);
		return false;
	}

	_owner->UpdateDisplayTitle(_current_title);
	return true;
}

bool player_title::SaveToDB(archive & ar)
{
	ar << _current_title;

	ar << _delivered_titles.size();
	if(!_delivered_titles.empty())  ar.push_back(_delivered_titles.begin(),_delivered_titles.size()*sizeof(TITLE_ID));

	ar << _expire_titles.size();

	// 因为数据对齐问题不能用	ar.push_back(_expire_titles.begin(),_expire_titles.size()*sizeof(TITLE_EXPIRE));
	for(size_t i = 0; i < _expire_titles.size(); ++i)
	{
		ar << _expire_titles[i].first;
		ar << _expire_titles[i].second;
	}

	return true;
}

void player_title::OnHeartbeat(int curtime)
{
	TITLE_EXPIRE_VEC::iterator iter = _expire_titles.begin();
	TITLE_EXPIRE_VEC::iterator iend = _expire_titles.end();
	
	for(;iter != iend; ++iter)
	{
		if((*iter).second <= curtime)
		{
			LoseTitle((*iter).first);
			return; // 一次Heartbeat只处理一个称号到期,避免出现临时数组
		}
	}

}

bool player_title::CalcAllTitleEnhance(bool updateplayer)
{
	TITLE_ID_VEC::iterator iter = _delivered_titles.begin();
	TITLE_ID_VEC::iterator iend = _delivered_titles.end();
	
	for(;iter != iend; ++iter)
	{
		const title_essence* title = world_manager::GetTitleMan().GetTitleEssence(*iter);	
	
		if(!title)
		{
			GLog::log(LOG_ERR,"roleid:%d calc enhance find a invalid title[%d] \n",_owner->_parent->ID.id, *iter);
			return false;
		}
		
		title->ActivateEssence(_owner);
	}

	if(updateplayer)
		property_policy::UpdatePlayer(_owner->GetPlayerClass(),_owner);	

	return true;
}

bool player_title::ObtainTitle(TITLE_ID tid,int expire)
{
	if(IsExistTitle(tid))
	{
		GLog::log(LOG_ERR,"roleid:%d obtain a already exist title[%d] \n",_owner->_parent->ID.id, tid);
		return false;
	}
	
	const title_essence* title = world_manager::GetTitleMan().GetTitleEssence(tid);	
	
	if(!title) 
	{
		GLog::log(LOG_ERR,"roleid:%d obtain a invalid title[%d] \n",_owner->_parent->ID.id, tid);
		return false;
	}

	title->ActivateEssence(_owner);

	property_policy::UpdatePlayer(_owner->GetPlayerClass(),_owner);	

	_delivered_titles.push_back(tid);
	
	if(expire)
	{
		if(!world_manager::GetTitleMan().IsSubTitle(tid)) // 策划可以保证次现象不会发生
		{
			expire += g_timer.get_systime();
			_expire_titles.push_back(abase::pair<TITLE_ID,int>(tid,expire));
		}
		else
		{
			GLog::log(LOG_ERR,"roleid:%d obtain a invalid expire title[%d] time[%d]\n",_owner->_parent->ID.id, tid, expire);
			expire = 0;
		}
	}

	_owner->_runner->notify_title_modify(tid, expire,1);

	if(title->IsRare())	_owner->OnObtainRareTitle(tid);

	GLog::log(LOG_INFO,"roleid:%d obtain title[%d] time[%d]\n",_owner->_parent->ID.id, tid, expire);

	return 	world_manager::GetTitleMan().TouchSuperTitle(this,tid);
}

bool player_title::LoseTitle(TITLE_ID tid)
{
	if(!IsExistTitle(tid))
	{
		GLog::log(LOG_ERR,"roleid:%d lose a no exist title[%d] \n",_owner->_parent->ID.id, tid);
		return false;
	}

	const title_essence* title = world_manager::GetTitleMan().GetTitleEssence(tid);	
	
	if(!title) 
	{
		GLog::log(LOG_ERR,"roleid:%d lose a invalid title[%d] \n",_owner->_parent->ID.id, tid);
		return false;
	}

	title->DeactivateEssence(_owner);

	property_policy::UpdatePlayer(_owner->GetPlayerClass(),_owner);
	
	EraseVectorElement(_delivered_titles,CheckIdFunctor(tid));
	EraseVectorElement(_expire_titles,CheckExpireFunctor(tid));

	_owner->_runner->notify_title_modify(tid,0,0);
	
	if(_current_title == tid)
	{
		ChangeCurrTitle(0);
	}

	GLog::log(LOG_INFO,"roleid:%d lose title[%d] \n",_owner->_parent->ID.id, tid);
	return true;
}

bool player_title::ChangeCurrTitle(TITLE_ID tid)
{
	if(tid && !IsExistTitle(tid))
	{
		GLog::log(LOG_ERR,"roleid:%d change a no exist title[%d] \n",_owner->_parent->ID.id, tid);
		return false;
	}

	if(tid == _current_title)
		return true;

	_current_title = tid;
	_owner->UpdateDisplayTitle(_current_title);
	_owner->_runner->notify_curr_title_change(_owner->_parent->ID.id,_current_title);

	GLog::log(LOG_DEBUG,"title[%d] change\n",tid);
	return true;
}

bool player_title::QueryTitleData(dispatcher* runner)
{
	packet_raw_wrapper ar;
	for(size_t i = 0; i < _expire_titles.size(); ++i)
	{
		ar << _expire_titles[i].first;
		ar << _expire_titles[i].second;
	}

	runner->query_title(_owner->_parent->ID.id,_delivered_titles.size(),_expire_titles.size(),
			_delivered_titles.begin(),_delivered_titles.size()*sizeof(TITLE_ID),ar.data(),ar.size());
	
	return true;
}

bool player_title::IsExistTitle(TITLE_ID tid)
{
	TITLE_ID_VEC::iterator iter = _delivered_titles.begin();
	TITLE_ID_VEC::iterator iend = _delivered_titles.end();
	
	for(;iter != iend; ++iter) // 有必要时加映射，目前循环查找
	{
		if(*iter == tid)
			return true;
	}

	return false; 
}
