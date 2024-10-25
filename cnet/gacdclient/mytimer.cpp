
#include "mytimer.hpp"
#include "commander.hpp"

namespace GNET
{

MyTimer MyTimer::instance;

MyTimer::MyTimer()
{
	Run();
}
void MyTimer::Run()
{
	// single thread !!
	Commander::GetInstance()->TimerUpdate();
	//for(size_t i = 0; i < GAntiCheaterClient::GetInstance()->GetServerCount(); ++i)
	//	GAntiCheaterClient::GetInstance(i)->DoTimer();
    Thread::HouseKeeper::AddTimerTask(this,0);
}


};

