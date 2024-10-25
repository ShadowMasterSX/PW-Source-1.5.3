
#include "dbmappasswordload.hrp"
#include "dbmappasswordsave.hrp"
#include "mappasswd.h"

#define DB_MAPPASSWORD_UPDATE_TIME 600 // 10min

namespace GNET
{

static int GetMapPasswordDataSize(const MapPasswordData& data)
{
    return (data.username.size() + 
            sizeof(data.value.userid) +
            sizeof(data.value.algorithm) +
            data.value.password.size() +
            data.value.matrix.size() +
            data.value.seed.size() +
            data.value.pin.size() +
            sizeof(data.value.rtime) +
            sizeof(data.value.refreshtime));
}

bool Passwd::GetMapPasswordDataNoLock(std::vector<MapPasswordData>& data, Octets& handle)
{
    Map::const_iterator iter = map.begin();
    Map::const_iterator iter_end = map.end();

    if (handle.size() != 0)
        iter = map.find(handle);

    int size = 0;
    int total = 0;

    for (; iter != iter_end; ++iter)
    {
        if (iter->second.GetRefreshTime() < updatetime)
            continue;

        if ((total >= 1024) || (size >= 131072))
        {
            handle = iter->first;
            return false;
        }

        MapPasswordData userdata;
        userdata.username = iter->first;
        userdata.value = iter->second;

        data.push_back(userdata);
        size += GetMapPasswordDataSize(userdata);
        ++total;
    }

    return true;
}

bool Passwd::Initialize()
{
    IntervalTimer::AddTimer(this, DB_MAPPASSWORD_UPDATE_TIME);
    return true;
}

bool Passwd::Update()
{
    if (initialized)
    {
        Thread::Mutex::Scoped l(locker);
        std::vector<MapPasswordData> data;

        Octets handle;
        bool finish = false;

        for (; !finish; )
        {
            finish = (GetMapPasswordDataNoLock(data, handle) || (data.size() == 0));

            if (data.size() != 0)
            {
                GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBMAPPASSWORDSAVE, DBMapPasswordSaveArg(data)));
                data.clear();
            }
        }

        updatetime = time(NULL);
    }

    return true;
}

void Passwd::OnDBConnect(Protocol::Manager* manager, int sid)
{
    if (!initialized)
    {
        manager->Send(sid, Rpc::Call(RPC_DBMAPPASSWORDLOAD, DBMapPasswordLoadArg()));
    }
}

void Passwd::OnDBLoad(const std::vector<MapPasswordData>& data, bool finish)
{
    if (!initialized)
    {
        Thread::Mutex::Scoped l(locker);
        std::vector<MapPasswordData>::const_iterator iter = data.begin();
        std::vector<MapPasswordData>::const_iterator iter_end = data.end();

        for (; iter != iter_end; ++iter)
        {
            map[iter->username] = iter->value;
        }

        initialized = finish;
    }
}

}

