
#include "timermanager.hpp"
#include "asmcodelibrary.hpp"
#include "stringcache.hpp"
#include "punishmanager.hpp"
#include "userdatamanager.hpp"
#include "queryproxy.hpp"

namespace GNET
{

TimerManager TimerManager::s_instance;

TimerManager::TimerManager()
{
	Run();
}

void TimerManager::Run()
{
	
	UserCodeManager::UpdateCodeSeq();
    UserDataManager::GetInstance()->OnTimer();
	StringCache::GetInstance()->OnTimer();
	AsmCodeLibrary::GetInstance()->OnTimer();
	PunishManager::GetInstance()->OnTimer();
	QueryProxy::GetInstance()->OnTimer();

    Thread::HouseKeeper::AddTimerTask(this,0);
}


};

