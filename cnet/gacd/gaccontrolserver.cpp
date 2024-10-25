
#include "gacdutil.h"
#include "gaccontrolserver.hpp"
#include "state.hxx"
#include "acconstants.h"
#include "acconnectre.hpp"
#include "xmlconfig.h"

namespace GNET
{

GACControlServer GACControlServer::instance;

const Protocol::Manager::Session::State* GACControlServer::GetInitState() const
{
	return &state_ACControlServer;
}

void GACControlServer::OnAddSession(Session::ID sid)
{
	//TODO
}

void GACControlServer::OnDelSession(Session::ID sid)
{
	//TODO
	Thread::Mutex::Scoped l(clients_locker);
    control_sid = (unsigned int)-1;
}

bool GACControlServer::SendProtocol(const Protocol *pro)
{
    Thread::Mutex::Scoped l(clients_locker);
    return Send(control_sid, pro);
}

bool GACControlServer::SendProtocol(const Protocol &pro)
{
    Thread::Mutex::Scoped l(clients_locker);
    return Send(control_sid, pro);
}

bool GACControlServer::SendProtocol(const Protocol *pro, Session::ID sid)
{
    return Send(sid, pro);
}

bool GACControlServer::SendProtocol(const Protocol &pro, Session::ID sid)
{
    return Send(sid, pro);
}

void GACControlServer::SetControlSID(Session::ID sid)
{
    Thread::Mutex::Scoped l(clients_locker);
    if( control_sid != (unsigned int)-1 )
    {
        DEBUG_PRINT_INIT("multi control client?\n");
        Close(sid);
        return;
    }
    control_sid = sid;
    DEBUG_PRINT_INIT("client connected!\n");
    ACConnectRe re;
    std::string str_aid = Conf::GetInstance()->find("Other", "zoneid");
    re.aid = atoi(str_aid.c_str());
    SendProtocol(re, sid);

}
/*
void GACControlServer::CodeRes(const StatManager::QueryResult &res, int rescode, std::string code_name, int roleid)
{
    if( code_name == "cpuid" )
    {
        ACCPUInfo pro_re;
        pro_re.roleid = roleid;
        if( -1 == roleid )
        {
            for(StatManager::QueryResult::const_iterator it = res.begin();
                    it != res.end(); ++it)
            {
                if( (*it).second.first == AC_CODE_RES_OK )
                {
                    const std::vector<std::pair<short,int> > &v = (*it).second.second;
                    if( v.size() == 8 )
                    {
                        std::vector<int> v2;
                        for(size_t i = 0; i<8; ++i)
                        {
                            v2.push_back(v[i].second);
                        }
                        pro_re.cpu_info[v2]++;
                    }
                }
            }
        }
        else
        {

            if( res.size() == 1 )
            {
                StatManager::QueryResult::const_iterator it = res.begin();
                if( (*it).first == roleid && (*it).second.first == AC_CODE_RES_OK )
                {
                    const std::vector<std::pair<short,int> > &v = (*it).second.second;
                    if( v.size() == 8 )
                    {
                        std::vector<int> v2;
                        for(size_t i = 0; i<8; ++i)
                        {
                            v2.push_back(v[i].second);
                        }
                        pro_re.cpu_info[v2] = 1;
                    }
                }
            }

        }
        SendProtocol(pro_re, control_sid);
    }
    else
    {
        ACCodeRes re;
        re.rescode = rescode;
        re.coderes = res;
        SendProtocol(re, control_sid);
    }
}
*/
};
