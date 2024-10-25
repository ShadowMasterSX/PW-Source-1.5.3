
#include "conf.h"
#include "login.h"
#include "security.h"
#include "timer.h"
#include <stdio.h>

using namespace GNET;

static Protocol::Type _state[] = {
PROTOCOL_CHALLENGE,
PROTOCOL_RESPONSE
};

static Protocol::Manager::Session::State state(_state, sizeof(_state)/sizeof(Protocol::Type), 5);

class ClientManager : public Protocol::Manager
{
	std::string Identification() const { return "Client"; }
	void OnAddSession(Session::ID sid)
	{
	printf("OnAddSession %d\n", sid);
	}

	void OnDelSession(Session::ID sid)
	{
	printf("OnDelSession %d\n", sid);
	}

	int PriorPolicy(Protocol::Type type) const
	{
		return 0;
	}

	bool InputPolicy(Protocol::Type type, size_t len) const
	{
	printf("Input Policy %d %d\n", type, len);
		return true;
	}
	const Session::State *GetInitState() const
	{
		return &state;
	}
public:
	std::string identity;
	std::string password;
	ClientManager(const char *name, const char *pwd) : identity(name), password(pwd) { }
};

static ClientManager manager("zengpan", "hello");

void Challenge::Process(Manager *man, Manager::Session::ID sid)
{
	ClientManager *manager = (ClientManager *)man;
	Response *response = (Response *)Protocol::Create(PROTOCOL_RESPONSE);
	response->Setup(manager->identity.c_str(), manager->password.c_str(), this);
	manager->Send(sid, response);
	response->Destroy();
}
void Response::Process(Manager *manager, Manager::Session::ID sid) {  }


int main()
{
	Conf::GetInstance("io.conf");
	Protocol::Client(&manager);
	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	Thread::Pool::Run();
	return 0;
}

