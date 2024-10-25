
#include "storage.h"
#include "gamedbserver.hpp"
#include "localmacro.h"
#include "state.hxx"

namespace GNET
{

GameDBServer GameDBServer::instance;
CrsLogicuidManager CrsLogicuidManager::_instance;

void CrsLogicuidManager::FindFreeLogicuid()
{
	int count = 0;
	int firstid = 0;
	size_t sizelogic=0;
	{
		Thread::Mutex::Scoped lock(mutex_);
		if(busy_)
			return;
		busy_ = true;
		if(idset_.size())
			firstid = idset_.front();
		count = 256 - idset_.size();
	}
	clock_t start = clock();
	std::vector<int> list;
	try
	{
		StorageEnv::Storage * plogicuid = StorageEnv::GetStorage("crslogicuid");
		StorageEnv::CommonTransaction txn;

		Marshal::OctetsStream key_default;
		sizelogic = plogicuid->count();
		key_default << 0;
		if(!startid_)
		{
			Marshal::OctetsStream value;
			if(plogicuid->find(key_default, value, txn))
				value >> startid_;
			else
				startid_ = LOGICUID_START;

			startid_ = LOGICUID(startid_);
		}
		for(int i=0; count>0 && startid_<LOGICUID_MAX && i<4096; startid_+=16,i++)
		{
			Marshal::OctetsStream value, value_logic;
			if(plogicuid->find(Marshal::OctetsStream()<<startid_, value_logic, txn))
				continue;
			count--;
			list.push_back(startid_);
		}
		if(!firstid && list.size())
			firstid = list.front();
		if(startid_>=LOGICUID_MAX)
			firstid = LOGICUID_START; // Largest logicuid reached, seek from beginning
		if(firstid)
			plogicuid->insert(key_default, Marshal::OctetsStream()<<firstid, txn);
	}
	catch ( DbException e )
	{
		Log::log(LOG_ERR,"CrsLogicuidManager::Run: what=%s.", e.what());
	}
	catch ( ... )
	{
		Log::log(LOG_ERR,"CrsLogicuidManager::Run: unknown error.");
	}

	clock_t used = clock()-start;
	Log::formatlog("crslogicuid","logicuid=%d:firstid=%d:idsfind=%d:timeused=%.3f",
			sizelogic,firstid,list.size(),(float)used/CLOCKS_PER_SEC);
	Thread::Mutex::Scoped lock(mutex_);
	idset_.insert(idset_.end(), list.begin(), list.end());
	busy_ = false;
	return;
}

int CrsLogicuidManager::AllocLogicuid()
{
	bool doseek = false;
	int id = 0;
	{
		Thread::Mutex::Scoped lock(mutex_);
		if(idset_.size())
		{
			id = idset_.front();
			idset_.erase(idset_.begin());
		}
		if(idset_.size()<=128)
			doseek = true;
	}
	if(doseek)
		Thread::Pool::AddTask(new CrsLogicuidSeeker());
	return id;
}

void CrsLogicuidSeeker::Run()
{
	CrsLogicuidManager::GetInstance()->FindFreeLogicuid();
	delete this;
}

const Protocol::Manager::Session::State* GameDBServer::GetInitState() const
{
	return &state_GameDBServer;
}

void GameDBServer::OnAddSession(Session::ID sid)
{
	Log::log( LOG_INFO, "GameDBServer::OnAddSession, sid=%d.\n", sid );
}

void GameDBServer::OnSetTransport(Session::ID sid, const SockAddr& local, const SockAddr& peer)
{
	Log::formatlog("addsession","sid=%d:ip=%s", sid, inet_ntoa(((const struct sockaddr_in*)peer)->sin_addr));
	SetSessionIP(sid,((const struct sockaddr_in*)peer)->sin_addr.s_addr);
}

void GameDBServer::OnDelSession(Session::ID sid)
{
	Log::log( LOG_INFO, "GameDBServer::OnDelSession, sid=%d.\n", sid );
	if ( sid==delivery_sid )
	{
		delivery_sid=0;
		Log::log( LOG_ERR, "GameDBServer::disconnect from delivery, sid=%d.\n", sid );
	}
}

};
