
#include <map>
#include "conf.h"
#include "login.h"
#include "thread.h"
#include "timer.h"

#include "timermanager.h"
#include "compress.h"

using namespace GNET;

struct UserTracker
{
	Octets name;
	Octets challenge;
	size_t size;
};

static Protocol::Type _state[] = {
PROTOCOL_CHALLENGE,
PROTOCOL_RESPONSE
};

static Protocol::Manager::Session::State state(_state, sizeof(_state)/sizeof(Protocol::Type), 5);


class ServerManager : public Protocol::Manager
{
	std::string Identification() const { return "Server"; }
	typedef std::map<Session::ID, UserTracker> UserMap;
	UserMap umap;
	Thread::RWLock locker_umap;

	void OnAddSession(Session::ID sid)
	{
		Challenge *challenge = (Challenge *)Protocol::Create(PROTOCOL_CHALLENGE);
		challenge->Setup(16);
		{
			Thread::RWLock::WRScoped l(locker_umap);
			UserMap::iterator it = umap.insert(std::make_pair(sid, UserTracker())).first;
			(*it).second.challenge = *challenge;
		}
		Send(sid, challenge);
		challenge->Destroy();
	}

	void OnDelSession(Session::ID sid)
	{
		Thread::RWLock::WRScoped l(locker_umap);
		umap.erase(sid);
	}

	int PriorPolicy(Protocol::Type type) const { return 0; }
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
	Octets *GetChallenge(Session::ID sid)
	{
		UserMap::iterator it = umap.find(sid);
		return it == umap.end() ? NULL : &(*it).second.challenge;
	}
};

static char *passwd = "hello";

void Challenge::Process(Manager *manager, Manager::Session::ID sid) {  }
void Response::Process(Manager *man, Manager::Session::ID sid)
{
	printf("Process %.*s\n", identity.size(), (char *)identity.begin());
	ServerManager *manager = (ServerManager *)man;
	HMAC_MD5Hash hash;
	Octets digest;
	hash.SetParameter(Octets(passwd, strlen(passwd)));
	Octets *challenge = manager->GetChallenge(sid);
	hash.Update(*challenge);
	hash.Final(digest);
	if (digest != response)
		manager->Close(sid);
}

static ServerManager manager;

int main()
{
	Conf::GetInstance("io.conf");
	Protocol::Server(&manager);
	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	Thread::Pool::Run();
	return 0;
}

