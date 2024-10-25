#ifndef __GNET_LOGSERVICETCPSERVER_HPP
#define __GNET_LOGSERVICETCPSERVER_HPP

#include "timer.h"
#include "protocol.h"

namespace GNET
{

class LogserviceTcpServer : public Protocol::Manager, public Timer::Observer
{
	static LogserviceTcpServer instance;
	size_t		accumulate_limit;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
public:
	static LogserviceTcpServer *GetInstance() { return &instance; }
	std::string Identification() const { return "LogserviceTcpServer"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	LogserviceTcpServer() : accumulate_limit(0) { Timer::Attach(this); }

	void Update( )
	{
		CheckSessionTimePolicy();
	}
};

};
#endif
