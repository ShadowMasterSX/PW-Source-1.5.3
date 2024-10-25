#ifndef __GNET_LOGSERVICESERVER_HPP
#define __GNET_LOGSERVICESERVER_HPP

#include "timer.h"
#include "protocol.h"

namespace GNET
{

class LogserviceServer : public Protocol::Manager, public Timer::Observer
{
	static LogserviceServer instance;
	size_t		accumulate_limit;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
public:
	static LogserviceServer *GetInstance() { return &instance; }
	std::string Identification() const { return "LogserviceServer"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	LogserviceServer() : accumulate_limit(0) { Timer::Attach(this); }

	void Update( )
	{
		CheckSessionTimePolicy();
	}

};

};
#endif
