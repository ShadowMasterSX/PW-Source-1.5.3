#ifndef __GNET_GACCONTROLSERVER_HPP
#define __GNET_GACCONTROLSERVER_HPP

#include "protocol.h"
#include "macros.h"
//#include "statmanager.hpp"


namespace GNET
{

class GACControlServer : public Protocol::Manager
{
	static GACControlServer instance;
	size_t		accumulate_limit;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	Thread::Mutex clients_locker;
    Session::ID control_sid;
public:
	static GACControlServer *GetInstance() { return &instance; }
	std::string Identification() const { return "GACControlServer"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GACControlServer() : accumulate_limit(0), clients_locker("gaccontrolserver locker"), control_sid((unsigned int)-1) { }
	bool SendProtocol(const Protocol* pro);
    bool SendProtocol(const Protocol& pro);
    bool SendProtocol(const Protocol* pro, Session::ID sid);
    bool SendProtocol(const Protocol& pro, Session::ID sid);
    void SetControlSID(Session::ID sid);

    //void CodeRes(const StatManager::QueryResult &res, int rescode, std::string code_name, int roleid);
};

};
#endif
