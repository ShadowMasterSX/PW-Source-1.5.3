
#include "uniquenameserver.hpp"
#include "state.hxx"
#include <time.h>

namespace GNET
{

UniqueNameServer  UniqueNameServer::instance;
LogicuidManager   LogicuidManager::instance;

const Protocol::Manager::Session::State* UniqueNameServer::GetInitState() const
{
	return &state_UniqueNameServer;
}

void UniqueNameServer::OnSetTransport(Session::ID sid, const SockAddr& local, const SockAddr& peer)
{
	Log::formatlog("addsession","sid=%d:ip=%s", sid, inet_ntoa(((const struct sockaddr_in*)peer)->sin_addr));
}

void UniqueNameServer::OnAddSession(Session::ID sid)
{
	LOG_TRACE( "OnAddSession sid=%d", sid );
}

void UniqueNameServer::OnDelSession(Session::ID sid)
{
	LOG_TRACE( "OnDelSession sid=%d", sid );
}
void UniqueNameServer::TransformName(Octets& name)
{
	if(!case_insensitive)
		return;
	unsigned char* data = (unsigned char*)name.begin();
	size_t size = name.size();
	for(size_t i=0;i<size-1;i+=2,data+=2)
	{
		if(*(data+1)==0 && *data>='A' && *data<='Z')
			*data += 'a'-'A';
	}
}

size_t LogicuidManager::FindFreeLogicuid()
{
	int count = 0;
	unsigned int firstid = 0;
	size_t sizelogic=0,sizeuser=0;
	{
		Thread::Mutex::Scoped lock(locker);
		if(busy)
			return 0;
		busy = true;
		if(idset.size())
			firstid = idset.front();
		count = 256 - idset.size();
	}
	clock_t start = clock();
	std::vector<unsigned int> list;
	try
	{

		StorageEnv::Storage * plogicuid = StorageEnv::GetStorage("logicuid");
		StorageEnv::Storage * puidrole  = StorageEnv::GetStorage("uidrole");
		StorageEnv::CommonTransaction txn;

		Marshal::OctetsStream key_default;
		sizelogic = plogicuid->count();
		sizeuser  = puidrole->count();
		key_default << 0;
		if(!startid)
		{
			Marshal::OctetsStream value;
			if(plogicuid->find(key_default, value, txn))
			{
				value >> startid;
				if(startid >= LOGICUID_MAX) startid = LOGICUID_START;
			}
			else
				startid = LOGICUID_START;

			startid = LOGICUID(startid);
		}
		for(int i=0; count>0 && startid<LOGICUID_MAX && i<4096; startid+=16,i++)
		{
			Marshal::OctetsStream value, value_logic;
			if(plogicuid->find(Marshal::OctetsStream()<<startid, value_logic, txn))
				continue;
			if(puidrole->find(Marshal::OctetsStream()<<startid, value, txn))
			{
				unsigned int roles = 0, logicuid = 0;
				if(value.size()==8)
					value >> roles >> logicuid;
				else
					continue;
				if(logicuid==startid)
					continue;
			}
			count--;
			list.push_back(startid);
		}
		if(!firstid && list.size())
			firstid = list.front();
		if(startid>=LOGICUID_MAX)
			firstid = LOGICUID_START; // Largest logicuid reached, seek from beginning
		if(firstid)
			plogicuid->insert(key_default, Marshal::OctetsStream()<<firstid, txn);
	}
	catch ( DbException e )
	{
		Log::log(LOG_ERR,"FindFreeLogicuid: what=%s.", e.what());
	}
	catch ( ... )
	{
		Log::log(LOG_ERR,"FindFreeLogicuid: unknown error.");
	}

        clock_t used = clock()-start;
        Log::formatlog("logicuid","uidrole=%d:logicuid=%d:firstid=%d:idsfind=%d:timeused=%.3f",
                sizeuser,sizelogic,firstid,list.size(),(float)used/CLOCKS_PER_SEC);
	Thread::Mutex::Scoped lock(locker);
	idset.insert(idset.end(), list.begin(), list.end());
	busy = false;
	return idset.size();
}
unsigned int  LogicuidManager::AllocLogicuid()
{
	bool doseek = false;
	unsigned int id = 0;
	{
		Thread::Mutex::Scoped lock(locker);
		if(idset.size())
		{
			id = idset.front();
			idset.erase(idset.begin());
		}
		if(idset.size()<=128)
			doseek = true;
	}
	if(doseek)
		Thread::Pool::AddTask(new LogicuidSeeker());
	return id;
}

void  LogicuidSeeker::Run()
{
	LogicuidManager::Instance().FindFreeLogicuid();
	delete this;
}
};
