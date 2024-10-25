

#include "codesender.hpp"
#include "usercodemanager.hpp"
#include "acqcoderes.hpp"
#include "gaccontrolserver.hpp"
#include "queryproxy.hpp"
#include "userdatamanager.hpp"


namespace GNET
{

void CodeResCheckerFromClient::DoCheck(WaitingCode *pWC, UserSessionData *pUser, int iRes, bool &bCommit)
{
    bCommit = false;
    QueryProxy::GetInstance()->CheckRes(m_iSessionID, pUser->GetUserID(), pWC->m_iCodeID, iRes);
}

QueryProxy QueryProxy::instance;

int QueryProxy::GetXID()
{
    Thread::Mutex::Scoped l(xid_locker);
    ++xid;
    if( xid > 65536*128 )
        xid = 1;
    return xid;
}

void QueryProxy::OnTimer()
{
	std::vector<int> remove_dummy;
	for(WaitingMap::iterator it = waiting_map.begin(), ie = waiting_map.end(); it != ie; ++it)
	{
		Waiting &w = (*it).second;
		if( --w.timeout <= 0 )
		{
			remove_dummy.push_back((*it).first);
			ACQCodeRes re;
			re.acq = w.acq;
			std::map<Res, int> tmap;
			for(std::map<int, Res>::const_iterator it2 = w.waiting.begin(), ie2 = w.waiting.end();
				it2 != ie2; ++it2)
				tmap[(*it2).second]++;
			for(std::map<Res, int>::const_iterator it2 = tmap.begin(), ie2 = tmap.end();	
				it2 != ie2; ++it2)
			{
				const Res &tres = (*it2).first;
				ACUserCodeRes acucr;
				acucr.count = (*it2).second;
				acucr.type = 1;
				for(std::vector<std::pair<short, int> >::const_iterator it3 = tres.res.begin(),
					ie3 = tres.res.end(); it3 != ie3; ++it3)
					acucr.res.push_back(ACCodeRes((*it3).first, (*it3).second));
				re.res.push_back(acucr);
			}
			GACControlServer::GetInstance()->SendProtocol(re, w.sid);
		}
	}
	for(std::vector<int>::const_iterator it = remove_dummy.begin(), ie = remove_dummy.end();
		it != ie; ++it)
	{
		waiting_map.erase((*it));
	}
}
void QueryProxy::CheckRes(int xid, int uid, short code_id, int res)
{
	WaitingMap::iterator it = waiting_map.find(xid);
	if( it == waiting_map.end() ) return;
	if( (*it).second.acq.roleid != -1 && (*it).second.acq.roleid != uid ) return;
	(*it).second.waiting[uid].Add(code_id, res);
}
void QueryProxy::SendQuery(ACQ acq, unsigned int sid, const IntOctetsVector &params)
{
	size_t n = params.size();
	if( n == 0 ) return;
	std::string code_name = std::string((const char*)(params[0].m_octets.begin()),
					params[0].m_octets.size());
	std::vector<std::string> code_params;
	for(size_t i=1; i<n; ++i)
		code_params.push_back(std::string((const char*)(params[i].m_octets.begin()),
					params[i].m_octets.size()));
	int newx = GetXID();
	CodeSender cs(code_name, code_params, new CodeResCheckerFromClient(newx));
	Waiting new_waiting;
	new_waiting.timeout = acq.timeout;
	new_waiting.sid = sid;
	new_waiting.acq = acq;
	waiting_map[newx] = new_waiting;
	if( acq.roleid == -1 )
		UserDataManager::GetInstance()->TravelOnline(&cs);
	else
		UserDataManager::GetInstance()->VisitOnline(acq.roleid, &cs);
}

};
